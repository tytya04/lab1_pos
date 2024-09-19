#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

void list_directory(const char *path, bool show_all, bool long_format) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    struct stat file_stat;

    printf("Listing files in directory: %s\n", path);

    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем записи "." и "..", если не установлен флаг -a
        if (!show_all && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        // Сформировать полный путь к файлу
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Получаем информацию о файле
        if (stat(full_path, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        // Проверяем формат, Права доступа, размер и имя файла/директории
        if (long_format) {
            printf((S_ISDIR(file_stat.st_mode)) ? "d" : "-");
            printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
            printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
            printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
            printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
            printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
            printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
            printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
            printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
            printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");
            printf(" %ld ", file_stat.st_size);
        }

        // Вывод имени файла/директории
        if (S_ISDIR(file_stat.st_mode)) {
            printf("[DIR]  %s\n", entry->d_name);
        } else {
            printf("[FILE] %s\n", entry->d_name);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *path = ".";  // По умолчанию использовать текущую директорию
    bool show_all = false;   // Флаг для -a (показывать скрытые файлы)

bool long_format = false; // Флаг для -l (длинный формат)

    // Обработка аргументов командной строки
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "ls") == 0) {
            continue; // Пропускаем команду "ls"
        }
        else if (strcmp(argv[i], "-a") == 0) {
            show_all = true; // Устанавливаем флаг для отображения скрытых файлов
        }
        else if (strcmp(argv[i], "-l") == 0) {
            long_format = true; // Устанавливаем флаг для длинного формата
        }
        else {
            // Если аргумент не является опцией, то это путь к директории
            path = argv[i];
        }
    }

    // Вызываем функцию для вывода содержимого директории
    list_directory(path, show_all, long_format);

    return EXIT_SUCCESS;
}