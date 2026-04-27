import os
import subprocess
import sys

# Директории, которые мы не хотим форматировать (например, сторонние либы или бинарники)
IGNORE_DIRS = {'build', '3rd', 'vendor', '.git', 'out', 'generated'}

# Расширения файлов, которые нужно форматировать
EXTENSIONS = {'.cpp', '.hpp', '.h', '.c', '.cc', '.cxx'}

def format_code(root_dir):
    files_to_format = []
    
    # 1. Рекурсивный поиск файлов
    for dirpath, dirnames, filenames in os.walk(root_dir):
        # Исключаем ненужные директории, чтобы os.walk в них не заходил
        dirnames[:] = [d for d in dirnames if d not in IGNORE_DIRS]
        
        for filename in filenames:
            ext = os.path.splitext(filename)[1].lower()
            if ext in EXTENSIONS:
                files_to_format.append(os.path.join(dirpath, filename))
                
    if not files_to_format:
        print("Файлы исходного кода не найдены.")
        return

    print(f"Найдено {len(files_to_format)} файлов для форматирования. Запускаю clang-format...")
    
    # 2. Форматирование файлов
    success_count = 0
    error_count = 0
    
    for filepath in files_to_format:
        try:
            # -i : изменять файл "на месте" (перезаписать)
            # -style=file : использовать конфигурацию из файла .clang-format
            subprocess.run(
                ['clang-format', '-i', '-style=file', filepath],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            success_count += 1
            print(f"Отформатирован: {filepath}")
        except subprocess.CalledProcessError as e:
            print(f"[ОШИБКА] Не удалось отформатировать {filepath}:")
            print(e.stderr.decode('utf-8', errors='ignore'))
            error_count += 1
        except FileNotFoundError:
            print("[КРИТИЧЕСКАЯ ОШИБКА] Утилита 'clang-format' не найдена в системе.")
            print("Убедитесь, что она установлена и добавлена в переменные среды PATH.")
            sys.exit(1)

    print("-" * 40)
    print("Процесс завершен!")
    print(f"Успешно: {success_count} файлов")
    if error_count > 0:
        print(f"С ошибками: {error_count} файлов")

if __name__ == '__main__':
    # Если передали путь через аргумент командной строки - используем его,
    # иначе форматируем корневую директорию проекта (на 1 уровень выше utils)
    if len(sys.argv) > 1:
        target_dir = sys.argv[1]
    else:
        # Скрипт находится в utils/, поэтому корень проекта это ../
        script_dir = os.path.dirname(os.path.abspath(__file__))
        target_dir = os.path.join(script_dir, "..")
        
    abs_target_dir = os.path.abspath(target_dir)
    print(f"Анализ директории: {abs_target_dir}")
    
    format_code(abs_target_dir)
