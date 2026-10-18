"""Build immutable runtime ZIP releases, optional UI atlases and overlay patches.

No upload or repository/source mutation. Paths in config are relative to config.
Pillow is needed only when atlas groups are used.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import re
import zipfile


def encode(value):
    return (json.dumps(value, ensure_ascii=False, sort_keys=True, indent=2) + "\n").encode("utf-8")


def logical(value):
    if not isinstance(value, str) or not value or "\\" in value or ":" in value or "\0" in value:
        raise ValueError(f"Invalid resource path: {value!r}")
    path = PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or str(path) == "." or path.parts[0].lower() in ("user", "cache"):
        raise ValueError(f"Invalid resource path: {value!r}")
    return str(path)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def read_json(path):
    return json.loads(Path(path).read_text(encoding="utf-8-sig"))


def validate_manifest(value):
    if value.get("schemaVersion") != 1 or not re.fullmatch(r"[a-zA-Z0-9_-]{1,80}", value.get("releaseId", "")):
        raise ValueError("Unsupported release schema or invalid ID")
    if not value.get("archives") or not isinstance(value.get("files"), dict):
        raise ValueError("Release is missing archives/files")
    for archive in value["archives"]:
        if not re.fullmatch("[0-9a-f]{64}", archive["sha256"]):
            raise ValueError("Invalid archive hash")
        logical(archive["path"])
        for deleted in archive.get("deletedPaths", []):
            logical(deleted)
    for name, value in value["files"].items():
        logical(name)
        if not re.fullmatch("[0-9a-f]{64}", value):
            raise ValueError("Invalid file hash")


def collect(config, directory):
    files = {}
    for name in config["roots"]:
        root = (directory / name).resolve()
        if not root.is_dir():
            raise ValueError(f"Missing asset root: {root}")
        for path in sorted(root.rglob("*")):
            if path.is_symlink():
                raise ValueError(f"Symlinks are not supported: {path}")
            if path.is_file():
                key = logical(path.relative_to(root).as_posix())
                files[key] = path.read_bytes()
    folded = {}
    for key in files:
        if key.casefold() in folded:
            raise ValueError(f"Case collision: {key}, {folded[key.casefold()]}")
        folded[key.casefold()] = key
    return files


def atlas_inputs(config, files):
    groups = {}
    active, visited = set(), set()

    def visit(name):
        name = logical(name)
        if name in active:
            raise ValueError(f"Resource pack cycle: {name}")
        if name in visited:
            return
        active.add(name)
        pack = json.loads(files[name])
        for child in pack.get("packs", []):
            visit(child)
        for kind, items in pack.get("resources", {}).items():
            for item in items:
                path = logical(item["path"].lstrip("/"))
                if path not in files:
                    raise ValueError(f"Missing resource in {name}: {path}")
                if item.get("atlas"):
                    if kind.lower() not in ("texture", "textures") or PurePosixPath(path).suffix.lower() not in (".png", ".jpg", ".jpeg", ".texture"):
                        raise ValueError("Atlas entries must be 2D PNG/JPEG textures or single-image .texture descriptors")
                    group = item["atlas"]
                    if group not in config.get("atlasGroups", {}):
                        raise ValueError(f"Unknown atlas group: {group}")
                    groups.setdefault(group, set()).add(path)
        active.remove(name)
        visited.add(name)

    for name in config.get("packs", []):
        visit(name)
    assigned = set()
    for names in groups.values():
        if assigned.intersection(names):
            raise ValueError("Texture belongs to multiple atlas groups")
        assigned.update(names)
    return groups


def texture_bytes(path, files):
    if PurePosixPath(path).suffix.lower() != ".texture":
        return files[path]
    descriptor = json.loads(files[path])
    if descriptor.get("TexType", "TEXTURE_2D") not in (0, "TEXTURE_2D") or descriptor.get("IsFloat", False) or descriptor.get("##parent"):
        raise ValueError(f"Unsupported atlas descriptor: {path}")
    if descriptor.get("UseMipmap", True) or descriptor.get("ColorData"):
        raise ValueError("Atlas descriptors require an image and UseMipmap=false")
    for name in ("WrapS", "WrapT"):
        if descriptor.get(name, "CLAMP_TO_EDGE") not in (1, "CLAMP_TO_EDGE"):
            raise ValueError("Atlas descriptors require CLAMP_TO_EDGE")
    for name in ("MinFilter", "MagFilter"):
        if descriptor.get(name, "LINEAR") not in (0, "LINEAR"):
            raise ValueError("Atlas descriptors require LINEAR filters")
    paths = descriptor.get("PathTexture", [])
    if len(paths) != 1:
        raise ValueError("Atlas descriptor must reference one image")
    source = logical(paths[0].lstrip("/"))
    if PurePosixPath(source).suffix.lower() not in (".png", ".jpg", ".jpeg"):
        raise ValueError("Atlas image must be PNG/JPEG")
    return files[source]


def make_atlases(config, files):
    mapping = {}
    groups = atlas_inputs(config, files)
    if not groups:
        return mapping
    from PIL import Image
    for group, names in sorted(groups.items()):
        if not re.fullmatch(r"[a-zA-Z0-9_-]+", group):
            raise ValueError("Invalid atlas group ID")
        options = config["atlasGroups"][group]
        size, padding = int(options.get("size", 2048)), int(options.get("padding", 2))
        if not 16 <= size <= 8192 or not 1 <= padding <= 32:
            raise ValueError("Invalid atlas size/padding")
        # Version 1 uses clamp/linear/sRGB RGBA and disables mipmaps to avoid
        # sampling neighboring regions. Other sampling contracts stay standalone.
        if options.get("mipmaps", False) or options.get("colorSpace", "srgb") != "srgb":
            raise ValueError("Atlas v1 requires srgb and mipmaps=false")
        pages = []
        image, rects = Image.new("RGBA", (size, size)), {}
        x = y = row = 0
        for name in sorted(names):
            with Image.open(io.BytesIO(texture_bytes(name, files))) as source:
                source.load()
                piece = source.convert("RGBA")
            w, h = piece.size
            if w + 2 * padding > size or h + 2 * padding > size:
                raise ValueError(f"Texture does not fit atlas: {name}")
            if x + w + 2 * padding > size:
                x, y, row = 0, y + row, 0
            if y + h + 2 * padding > size:
                pages.append((image, rects))
                image, rects = Image.new("RGBA", (size, size)), {}
                x = y = row = 0
            # Extrude edges/corners into padding using nearest neighbor.
            for py in range(-padding, h + padding):
                sy = min(h - 1, max(0, py))
                for px in range(-padding, w + padding):
                    if 0 <= px < w and 0 <= py < h:
                        continue
                    sx = min(w - 1, max(0, px))
                    image.putpixel((x + padding + px, y + padding + py), piece.getpixel((sx, sy)))
            image.paste(piece, (x + padding, y + padding))
            rects[name] = dict(X=x + padding, Y=y + padding, W=w, H=h)
            x += w + 2 * padding
            row = max(row, h + 2 * padding)
        pages.append((image, rects))
        for index, (image, rects) in enumerate(pages):
            path = f"atlases/{group}-{index}.png"
            metadata = str(PurePosixPath(path).with_suffix(".atlas"))
            if path in files or metadata in files:
                raise ValueError("Generated atlas collides with an input")
            output = io.BytesIO()
            image.save(output, format="PNG", optimize=False, compress_level=9)
            files[path] = output.getvalue()
            files[metadata] = encode({"Files": rects, "Path": path})
            for name in rects:
                mapping[name] = dict(kind="atlas", path=path, metadata=metadata, region=name)
        # Retain originals for 3D/material consumers that require standalone UVs.
    return mapping


def archive_bytes(files):
    output = io.BytesIO()
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_STORED, allowZip64=True) as archive:
        for name, data in sorted(files.items()):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.create_system = 3
            info.external_attr = 0o100644 << 16
            archive.writestr(info, data)
    return output.getvalue()


def build(config_path, output, previous=None):
    config_path, output = Path(config_path), Path(output)
    config = read_json(config_path)
    for name in config["roots"]:
        if output.resolve().is_relative_to((config_path.parent / name).resolve()):
            raise ValueError("Build output must be outside asset roots")
    if not re.fullmatch(r"[a-zA-Z0-9_-]{1,80}", config["releaseId"]):
        raise ValueError("Invalid release ID")
    files = collect(config, config_path.parent)
    mapping = make_atlases(config, files)
    if "resource-map.json" in files:
        raise ValueError("resource-map.json is reserved for generated output")
    files["resource-map.json"] = encode(dict(schemaVersion=1, textures=mapping))
    hashes = {name: sha(data) for name, data in sorted(files.items())}
    archives, deleted = [], []
    old = None
    if previous:
        old = read_json(previous)
        validate_manifest(old)
        if old["platform"] != config.get("platform", "any") or old["releaseId"] == config["releaseId"]:
            raise ValueError("Patch platform/release mismatch")
        archives = old["archives"].copy()
        deleted = sorted(set(old["files"]) - set(files))
        files = {name: data for name, data in files.items() if old["files"].get(name) != hashes[name]}
    data = archive_bytes(files)
    digest = sha(data)
    archive_path = f"archives/{digest}.zip"
    output.mkdir(parents=True, exist_ok=True)
    (output / "archives").mkdir(exist_ok=True)
    (output / archive_path).write_bytes(data)
    # Copy parent archives so every output directory is independently deployable.
    if previous:
        import shutil
        for item in archives:
            source = Path(previous).parent / item["path"]
            if source.resolve() != (output / item["path"]).resolve():
                shutil.copyfile(source, output / item["path"])
    archives.append(dict(path=archive_path, sha256=digest, size=len(data), deletedPaths=deleted))
    result = dict(schemaVersion=1, releaseId=config["releaseId"], platform=config.get("platform", "any"),
                  minEngineVersion=1, archives=archives, files=hashes)
    if old:
        result["fromRelease"] = old["releaseId"]
    validate_manifest(result)
    target = output / f"{result['releaseId']}.json"
    target.write_bytes(encode(result))
    verify(target)
    return target


def verify(manifest_path):
    manifest_path = Path(manifest_path)
    value = read_json(manifest_path)
    validate_manifest(value)
    visible = {}
    for item in value["archives"]:
        path = manifest_path.parent / item["path"]
        data = path.read_bytes()
        if len(data) != item["size"] or sha(data) != item["sha256"]:
            raise ValueError(f"Archive integrity mismatch: {path}")
        for deleted in item.get("deletedPaths", []):
            visible = {key: data for key, data in visible.items() if key != deleted and not key.startswith(deleted + "/")}
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            folded = set()
            for name in archive.namelist():
                key = logical(name)
                if key.casefold() in folded:
                    raise ValueError("Duplicate archive path")
                folded.add(key.casefold())
                visible[key] = archive.read(name)
    if {key: sha(data) for key, data in visible.items()} != value["files"]:
        raise ValueError("Release file catalog mismatch")
    mapping = json.loads(visible["resource-map.json"])
    for entry in mapping["textures"].values():
        if entry["path"] not in visible:
            raise ValueError("Unresolved texture mapping")
        if entry["kind"] == "atlas":
            atlas = json.loads(visible[entry["metadata"]])
            if entry["region"] not in atlas["Files"]:
                raise ValueError("Unresolved atlas region")
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    for command in ("build", "patch"):
        item = sub.add_parser(command)
        item.add_argument("config", type=Path)
        item.add_argument("--output", required=True, type=Path)
        if command == "patch":
            item.add_argument("--previous", required=True, type=Path)
    sub.add_parser("verify").add_argument("manifest", type=Path)
    args = parser.parse_args()
    if args.command == "verify":
        verify(args.manifest)
        print(f"Verified {args.manifest}")
    else:
        print(build(args.config, args.output, getattr(args, "previous", None)))


if __name__ == "__main__":
    main()
