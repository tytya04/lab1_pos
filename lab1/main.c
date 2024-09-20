#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/stat.h>   
#include <dirent.h>    
#include <pwd.h>        
#include <grp.h>        
#include <time.h>       
#include <stdbool.h>    
#include <string.h>     

// Максимальное количество файлов для обработки в директории
#define MAX_FILES 1024

// Структура для хранения информации о файле
typedef struct {
    char name[256];      // Имя файла
    struct stat st;      // информацию о файле
} FileEntry;

// Функция для сравнения имен файлов, используемая для сортировки
int compare(const void *a, const void *b) {
    return strcmp(((FileEntry *)a)->name, ((FileEntry *)b)->name);  // Сравниваем имена файлов в алфавитном порядке
}

// Функция для вывода содержимого директории
void list_directory(const char *path, bool show_all, bool long_format) {
    DIR *dir;  // Указатель на структуру
    struct dirent *entry;  // Структура для хранения информации о текущем файле/папке
    FileEntry files[MAX_FILES];  // Массив для хранения файлов из директории
    int count = 0;  // Счетчик для количества файлов

    // Открываем директорию
    dir = opendir(path);
    if (!dir) {  // Если директория не может быть открыта, выводим сообщение об ошибке
        perror("diropen");
        return;
    }

    // Чтение содержимого директории
    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем "." и "..", если флаг show_all не установлен
        if (!show_all && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        // Формируем полный путь к файлу
        // snprintf — записывает форматированную строку в массив
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // stat — это функция для получения информации о файле по его пути
        if (stat(full_path, &files[count].st) == -1) {
            perror("stat");
            continue;
        }

        // strncpy — это функция для копирования строк, она копирует указанное количество символов
        // В данном случае копируется имя файла в структуру files
        strncpy(files[count].name, entry->d_name, sizeof(files[count].name));
        count++;
    }

    closedir(dir); 

    // Сортируем массив файлов по имени
    qsort(files, count, sizeof(FileEntry), compare);

    // Выводим информацию о каждом файле
    for (int i = 0; i < count; i++) {
        if (long_format) {
            uid_t uid = getuid();
            struct group *gr = getgrgid(uid);
            struct passwd *pw = getpwuid(uid);
            char time[40];  // Буфер для хранения форматированного времени
            strftime(time, 40, "%b %d %H:%M", localtime(&(files[i].st.st_mtime))); // (буфер, размер_буфера, "формат", время)

            // Проверяем, является ли файл директорией и используем разные цвета для вывода
            if (S_ISDIR(files[i].st.st_mode)) {
                printf("\033[1;34m"); 
            }

            printf("%s%s%s%s%s%s%s%s%s%s %lu %5s %5s %lld %s %s\n", 
                (S_ISDIR(files[i].st.st_mode)) ? "d" : "-",  // Тип файла (d - директория, - файл)
                (files[i].st.st_mode & S_IRUSR) ? "r" : "-", // Права для владельца
                (files[i].st.st_mode & S_IWUSR) ? "w" : "-", 
                (files[i].st.st_mode & S_IXUSR) ? "x" : "-",
                (files[i].st.st_mode & S_IRGRP) ? "r" : "-", // Права для группы
                (files[i].st.st_mode & S_IWGRP) ? "w" : "-", 
                (files[i].st.st_mode & S_IXGRP) ? "x" : "-",
                (files[i].st.st_mode & S_IROTH) ? "r" : "-", // Права для остальных
                (files[i].st.st_mode & S_IWOTH) ? "w" : "-",
                (files[i].st.st_mode & S_IXOTH) ? "x" : "-",
                (unsigned long)files[i].st.st_nlink,  // Количество ссылок на файл
                pw->pw_name,  // Имя владельца файла
                gr->gr_name,  // Имя группы владельца файла
                (long long)files[i].st.st_size,  // Размер файла
                time,  // Время последней модификации
                files[i].name  // Имя файла
            );
            if (S_ISDIR(files[i].st.st_mode)) {
                printf("\033[0m");  // Сбрасываем цвет после вывода директорий
            }
        } else {
            // Если файл является директорией, подсвечиваем его синим цветом
            if (S_ISDIR(files[i].st.st_mode)) {
                printf("\033[1;34m%s\033[0m\n", files[i].name); 
            } else {
                printf("%s\n", files[i].name); 
            }
        }
    }
}


int main(int argc, char *argv[]) {
    const char *path = ".";  // По умолчанию используется текущая директория
    bool show_all = false;   // Флаг для -a (показывать скрытые файлы)
    bool long_format = false; // Флаг для -l (длинный формат вывода)

    // Проверяем, что первым аргументом передано 'ls'
    if (argc < 2 || strcmp(argv[1], "ls") != 0) {
        fprintf(stderr, "Usage: ./lab ls [options] [path]\n");
        return 1;
    }

    // Обрабатываем аргументы командной строки начиная со второго
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            show_all = true;  // Устанавливаем флаг для отображения скрытых файлов
        } else if (strcmp(argv[i], "-l") == 0) {
            long_format = true;  // Устанавливаем флаг для длинного формата
        } else if (strcmp(argv[i], "-la") == 0 || strcmp(argv[i], "-al") == 0) {
            show_all = true;
            long_format = true;  // Устанавливаем оба флага (-a и -l)
        } else {
            // Если аргумент не является опцией, то это путь к директории
            path = argv[i];
        }
    }

    list_directory(path, show_all, long_format);

    return 0;
}
