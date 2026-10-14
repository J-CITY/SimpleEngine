"""Install the pinned official Streamline x64 runtime (headers are a submodule)."""

# Support direct CLI invocation from any working directory.
if __package__ in (None, ""):
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parents[2]))

import hashlib
import json
import shutil
import subprocess
import zipfile
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[2]
VERSION = '2.14.1'
SHA256 = '92c4d954631a1710da86ca3fa8d5034f2b9503838c95fc4ae977ae149319781b'
URL = f'https://github.com/NVIDIA-RTX/Streamline/releases/download/v{VERSION}/streamline-sdk-v{VERSION}.zip'
INSTALL = ROOT / 'utils/.pack' / ('streamline-' + VERSION)
RUNTIME_FILES = tuple('bin/x64/' + name + '.dll' for name in (
    'sl.interposer', 'sl.common', 'sl.dlss', 'sl.dlss_g', 'sl.reflex', 'sl.pcl',
    'nvngx_dlss', 'nvngx_dlssg', 'NvLowLatencyVk'))


def file_sha256(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def runtime_ready(install=None):
    install = INSTALL if install is None else Path(install)
    try:
        manifest = json.loads((install / 'runtime-manifest.json').read_text(encoding='utf-8'))
        if manifest.get('version') != VERSION or manifest.get('archive_sha256') != SHA256:
            return False
        for relative in RUNTIME_FILES:
            dll = install / relative
            with dll.open('rb') as stream:
                if stream.read(2) != b'MZ':
                    return False
            if file_sha256(dll) != manifest.get('files', {}).get(relative):
                return False
        return True
    except (OSError, ValueError, TypeError, AttributeError):
        return False


def ensure_runtime(install=None, archive=None):
    install = INSTALL if install is None else Path(install)
    if runtime_ready(install):
        print(f'Streamline {VERSION} runtime is ready: {install}')
        return install
    archive = install.parent / ('streamline-' + VERSION + '.zip') if archive is None else Path(archive)
    archive.parent.mkdir(parents=True, exist_ok=True)
    if not archive.exists():
        partial = archive.with_suffix('.zip.partial')
        curl = shutil.which('curl.exe') or shutil.which('curl')
        if not curl:
            raise RuntimeError('curl is required to download the official Streamline runtime')
        print(f'Downloading official Streamline {VERSION} runtime...', flush=True)
        subprocess.run([curl, '--fail', '--location', '--silent', '--show-error', '--retry', '3',
                        '--output', str(partial), URL], check=True)
        partial.replace(archive)
    if file_sha256(archive) != SHA256:
        raise RuntimeError(f'Streamline release checksum mismatch: {archive}; remove the invalid archive and retry')
    with zipfile.ZipFile(archive) as bundle:
        count = 0
        for entry in bundle.infolist():
            relative = PurePosixPath(entry.filename)
            if relative.is_absolute() or '..' in relative.parts or '\\' in entry.filename or ':' in entry.filename:
                raise RuntimeError('Invalid path in SDK archive')
            if entry.is_dir():
                continue
            if any(part.lower() in ('bin', 'lib', 'include', 'licenses') for part in relative.parts) or 'license' in relative.name.lower():
                destination = install.joinpath(*relative.parts)
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(bundle.read(entry))
                count += 1
        print(f'Installed {count} SDK runtime files into {install}')
    files = {}
    for relative in RUNTIME_FILES:
        dll = install / relative
        with dll.open('rb') as stream:
            if stream.read(2) != b'MZ':
                raise RuntimeError(f'Streamline archive did not provide a runtime DLL: {relative}')
        files[relative] = file_sha256(dll)
    manifest = {'version': VERSION, 'archive_sha256': SHA256, 'files': files}
    (install / 'runtime-manifest.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
    return install

