#include <limits.h>  // Для PATH_MAX
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#define RESET "\033[0m"
#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define WHITE "\033[1;37m"
#define GREEN "\033[1;32m"
#define ORANGE "\033[38;5;214m"
#define PINK "\033[38;5;13m"

#define BUF_SIZE 1024

#define OK 0
#define ERROR 1

#define ARGS_SIZE 10;

using namespace std;

int changeDir(string s, int len_s, char *cur_dir, string home_dir);

int main() {
    printf("%sStart terminal...%s\n", ORANGE, RESET);

    string home_dir = getenv("HOME");  // Получаем путь к домашней директории

    if ((home_dir.c_str() == nullptr) || (chdir(home_dir.c_str()) != 0))
        perror("chdir() error");  // Переходим домашнюю директорию

    char cur_dir[PATH_MAX];  // Буфер для хранения пути

    while (true) {
        // Печатаем текущую директорию
        if (getcwd(cur_dir, sizeof(cur_dir)) != nullptr)
            printf("%s%s%s: ", GREEN, cur_dir, RESET);
        else {
            perror("getcwd() error");
            break;
        }

        string s = "";
        getline(cin, s);  // Считывам строку

        // Убираем пробелы в начале строки
        int i = 0;
        while (s[i] == ' ') s.erase(i, 1);

        // Убираем пробелы в конце строки
        i = s.size() - 1;
        while (s[i] == ' ') {
            s.erase(i, 1);
            i -= 1;
        }

        int cnt_arg = 0;  // Счетчик аргументов функции
        int args_size = ARGS_SIZE;
        string *args = (string *)malloc(args_size * sizeof(string));

        bool flag_single_quot = false;
        size_t ind_single = 0;
        bool flag_double_quot = false;
        size_t ind_double = 0;

        for (size_t i = 0; i < s.size(); i++)
            if (s[i] == '\'') {
                if (!flag_single_quot) {
                    ind_single = i;
                } else {
                    s.erase(i, 1);
                    s.erase(ind_single, 1);
                }
                flag_single_quot = !flag_single_quot;
            }
            else if (s[i] == '"') {
                if (!flag_double_quot) {
                    ind_double = i;
                } else {
                    s.erase(i, 1);
                    s.erase(ind_double, 1);
                }
                flag_double_quot = !flag_double_quot; 
            }
            else if (s[i] == ' ') {
                if (s[i - 1] != '\\' && !flag_single_quot && !flag_double_quot) cnt_arg += 1;  // Считаем аргументы функции
                while (i + 1 < s.size() && s[i + 1] == ' ') s.erase(i + 1, 1);  // Удаляем лишние пробелы
            }

        printf("<%s><%d>\n", s.c_str(), cnt_arg);

        int len_s = s.size();
        if (len_s == 0) continue;

        string command = strtok((char *)s.c_str(), " ");

        if (command == "exit") {
            break;
        } else if (command == "cd") {
            if (cnt_arg > 1)
                printf("cd: too many arguments\n");
            else
                changeDir(s, len_s, cur_dir, home_dir);
        } else {
            printf("%s%s%s\n", PINK, command.c_str(), RESET);
            system(s.c_str());
        }
    }
    printf("%sClose terminal...%s\n", RED, RESET);
    return 0;
}

int changeDir(string s, int len_s, char *cur_dir, string home_dir) {
    if (len_s > 3) {
        printf("<%ld>\n", strlen("cd"));
        string path = s.substr(strlen("cd") + 1, len_s);

        // Удаляем лишние '\'
        for (size_t i = 0; i < path.size(); i++)
            if (path[i] == '\\') path.erase(i, 1);

        // Переходим в новую директорию
        printf("%s%s%s\n", YELLOW, path.c_str(), RESET);
        if (chdir(path.c_str()) == 0)
            strcpy(cur_dir, path.c_str());  // в случае успеха меняем текущую директорию
        else
            printf("cd: no such directory: %s\n", path.c_str()); // сообщение об ошибке
    } else if (chdir(home_dir.c_str()) != 0) // переходим в домашнюю директорию после команды "cd"
        perror("chdir() error");

    return 0;
}
