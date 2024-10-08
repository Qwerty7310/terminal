#include <limits.h>  // Для PATH_MAX
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

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

using namespace std;

int changeDir(vector<string> args, char *cur_dir, string home_dir);

int main() {
    printf("%sStart terminal...%s\n", ORANGE, RESET);
    string home_dir = getenv("HOME");  // Получаем путь к домашней директории
    // Переходим в домашнюю директорию
    if ((home_dir.c_str() == nullptr) || (chdir(home_dir.c_str()) != 0)) 
        perror("chdir() error");  

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
        s = s + " ";  // добавляем пробел для корректой работы strtok при отстутствии аргументов

        string command = strtok((char *)s.c_str(), " ");  // получаем команду
        s[command.size()] = ' '; // меняем '\0' после strtok на пробел
        string str_args = " " + s.substr(command.size() + 1, s.size()) + " ";  // получаем строку аргументов

        // удаляем лишние пробелы
        i = 0;
        while (i + 1 < str_args.size() && str_args[i + 1] == ' ') str_args.erase(i + 1, 1);

        vector<string> args;  // вектор аргументов функции

        bool flag_double_quot = false;  // флаг открытия кавычек
        size_t ind_space = 0;           // индекс последнего пробела

        // разбиваем строку на отдельные аргументы
        for (size_t i = 0; i < str_args.size(); i++) {
            if (str_args[i] == '\\')
                str_args.erase(i, 1);  // удаляем символ '\', i теперь указывает на экранируемый символ, на
                                       // следующей итерации i++ перепрыгнет этот экранируемый символ
            else if (str_args[i] == '"') {
                flag_double_quot = !flag_double_quot;
                str_args.erase(i, 1);
                i -= 1;  // после удаления символы смещаются на 1 назад, необходимо вернуться на 1 символ
                         // назад, чтобы попасть на следующий символ
            } else if ((str_args[i] == ' ') && (!flag_double_quot)) {
                if (i - ind_space > 1) {
                    args.push_back(str_args.substr(ind_space + 1, i - (ind_space + 1)));
                }
                ind_space = i;
            }
        }

        //если не закрыты кавычки
        if (flag_double_quot) {
            printf("%s\b: incorrect use of quotation marks\n", s.c_str());
            continue;
        }

        // обрабатываем команды
        if (command == "close")
            break;
        else if (command == "cd") {
            if (args.size() > 1)
                printf("cd: too many arguments\n");
            else
                changeDir(args, cur_dir, home_dir);
        } else {
            printf("%s%s%s\n", PINK, command.c_str(), RESET);
            system(s.c_str());
        }
    }
    printf("%sClose terminal...%s\n", RED, RESET);

    return 0;
}

int changeDir(vector<string> args, char *cur_dir, string home_dir) {
    if (!args.empty()) {
        // Переходим в новую директорию
        if (chdir(args[0].c_str()) == 0)
            strcpy(cur_dir, args[0].c_str());  // в случае успеха меняем текущую директорию
        else
            printf("cd: no such directory: %s\n", args[0].c_str());  // сообщение об ошибке
    } else if (chdir(home_dir.c_str()) != 0)  // переходим в домашнюю директорию после команды "cd"
        perror("chdir() error");

    return 0;
}
