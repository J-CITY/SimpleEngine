import os
from pathlib import Path

ROOT_DIR = "."

CPP_EXTENSIONS = {
    ".cpp",
    ".hpp",
    ".h",
    ".cc",
    ".cxx",
}


def snake_to_camel_text(text: str) -> str:
    result = []
    i = 0

    while i < len(text):
        # нашли _
        if text[i] == "_" and i + 1 < len(text):
            next_char = text[i + 1]

            # если после _ буква
            if next_char.isalpha():
                result.append(next_char.upper())
                i += 2
                continue

        result.append(text[i])
        i += 1

    return "".join(result)


def process_file(filepath: Path):
    try:
        content = filepath.read_text(encoding="utf-8")
    except Exception as e:
        print(f"Ошибка чтения {filepath}: {e}")
        return

    new_content = snake_to_camel_text(content)

    if new_content != content:
        filepath.write_text(new_content, encoding="utf-8")
        print(f"Обработан: {filepath}")


def main():
    for root, _, files in os.walk(ROOT_DIR):
        for file in files:
            path = Path(root) / file

            if path.suffix in CPP_EXTENSIONS:
                process_file(path)


if __name__ == "__main__":
    main()