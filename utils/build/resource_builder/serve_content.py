"""Local release server with byte ranges; production can use any HTTPS file server."""
import argparse
import hashlib
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import re


class ContentHandler(SimpleHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def send_head(self):
        path = Path(self.translate_path(self.path))
        if not path.is_file():
            return super().send_head()
        size = path.stat().st_size
        with path.open("rb") as source:
            digest = hashlib.file_digest(source, "sha256").hexdigest() if hasattr(hashlib, "file_digest") else hashlib.sha256(source.read()).hexdigest()
        etag = f'"{digest}"'
        start, end = 0, size - 1
        request_range = self.headers.get("Range")
        partial = bool(request_range and self.headers.get("If-Range", etag) == etag)
        if partial:
            match = re.fullmatch(r"bytes=(\d+)-(\d*)", request_range)
            if not match:
                self.send_error(400, "Only one explicit byte range is supported")
                return None
            start = int(match[1])
            end = min(end, int(match[2])) if match[2] else end
            if start >= size or start > end:
                self.send_response(416)
                self.send_header("Content-Range", f"bytes */{size}")
                self.send_header("Content-Length", "0")
                self.end_headers()
                return None
        self.send_response(206 if partial else 200)
        self.send_header("Content-Type", self.guess_type(str(path)))
        self.send_header("Content-Length", str(max(0, end - start + 1)))
        self.send_header("Accept-Ranges", "bytes")
        self.send_header("ETag", etag)
        if partial:
            self.send_header("Content-Range", f"bytes {start}-{end}/{size}")
        self.end_headers()
        source = path.open("rb")
        source.seek(start)
        self.bytes_left = max(0, end - start + 1)
        return source

    def copyfile(self, source, outputfile):
        if not hasattr(self, "bytes_left"):
            return super().copyfile(source, outputfile)
        left = self.bytes_left
        del self.bytes_left
        while left:
            data = source.read(min(left, 64 * 1024))
            if not data:
                break
            outputfile.write(data)
            left -= len(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--bind", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()
    directory = args.directory.resolve()
    if not directory.is_dir():
        parser.error("Release directory is missing")
    def handler(*values, **options):
        return ContentHandler(*values, directory=str(directory), **options)
    with ThreadingHTTPServer((args.bind, args.port), handler) as server:
        print(f"Serving {directory} at http://{args.bind}:{server.server_port}", flush=True)
        server.serve_forever()


if __name__ == "__main__":
    main()
