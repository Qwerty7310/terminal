#include <limits.h>  // Для PATH_MAX
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <vector>

#define RESET "\033[0m"
#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define WHITE "\033[1;37m"
#define GREEN "\033[1;32m"
#define ORANGE "\033[38;5;214m"
#define PINK "\033[38;5;13m"
#define CYAN "\033[1;36m"

#define BUF_SIZE 1024

#define OK 0
#define ERROR 1

using namespace std;

vector<string> splitStr(string &str_input);
int changeDir(vector<string> args, string cur_dir, string home_dir);
string getHostName();

int main() {
    printf("%sStart terminal...%s\n", ORANGE, RESET);
    string home_dir = getenv("HOME");  // Получаем путь к домашней директории
    string cur_user = getenv("USER");  // получаем имя пользователя
    string host_name = getHostName();  // получаем имя системы

    // Переходим в домашнюю директорию
    if ((home_dir.c_str() == nullptr) || (chdir(home_dir.c_str()) != 0)) perror("chdir() error");

    string cur_dir;  // Буфер для хранения пути

    while (true) {
        // Печатаем текущую директорию
        cur_dir = filesystem::current_path();
        if (!cur_dir.empty()) {
            printf("%s%s@%s%s:", GREEN, cur_user.c_str(), host_name.c_str(), RESET);
            if (cur_dir.find(home_dir) == 0)
                printf("%s~%s%s$ ", CYAN, cur_dir.substr(home_dir.size(), cur_dir.size()).c_str(), RESET);
            else
                printf("%s%s%s$ ", CYAN, cur_dir.c_str(), RESET);
        } else {
            perror("getcwd() error");
            break;
        }

        // ввод строки
        string str_input;
        getline(cin, str_input);

        vector<string> args =
            splitStr(str_input);  // разделяем строку на отдельные аргументы и записываем в вектор

        // если вектор пуст
        if (args.empty()) continue;  // переходим на новую итерацию

        // обрабатываем команды
        if (args[0] == "close")
            break;
        else if (args[0] == "cd") {
            if (args.size() > 2)
                printf("cd: too many arguments\n");
            else
                changeDir(args, cur_dir, home_dir);
        } else {
            system(str_input.c_str());

            // pid_t pid = fork();

            // if (pid == 0) {
            //     system(str_input.c_str());
            // } else {
            //     printf("Parent: %d\n", pid);
            // }
        }
    }
    printf("%sClose terminal...%s\n", RED, RESET);

    return 0;
}

vector<string> splitStr(string &str_input) {
    vector<string> args;  // вектор аргументов

    // Убираем пробелы в начале строки
    size_t i = 0;
    while (str_input[i] == ' ') str_input.erase(i, 1);

    // Убираем пробелы в конце строки
    i = str_input.size() - 1;
    while (str_input[i] == ' ') {
        str_input.erase(i, 1);
        i -= 1;
    }
    str_input = " " + str_input + " ";  // добавляем пробел в начало и конец для корректой работы
    if (str_input == "  ") return vector<string>();  // возвращаем пустой вектор

    // удаляем лишние пробелы
    i = 0;
    while (i + 1 < str_input.size() && str_input[i + 1] == ' ') str_input.erase(i + 1, 1);

    bool flag_double_quot = false;  // флаг открытия кавычек
    size_t ind_space = 0;           // индекс последнего пробела

    // разбиваем строку на отдельные аргументы
    for (size_t i = 0; i < str_input.size(); i++) {
        if (str_input[i] == '\\')
            str_input.erase(i, 1);  // удаляем символ '\', i теперь указывает на экранируемый символ, на
                                    // следующей итерации i++ перепрыгнет этот экранируемый символ
        else if (str_input[i] == '"') {
            flag_double_quot = !flag_double_quot;
            str_input.erase(i, 1);
            i -= 1;  // после удаления символы смещаются на 1 назад, необходимо вернуться на 1 символ
                     // назад, чтобы попасть на следующий символ
        } else if ((str_input[i] == ' ') && (!flag_double_quot)) {
            if (i - ind_space > 1) {
                args.push_back(str_input.substr(ind_space + 1, i - (ind_space + 1)));
            }
            ind_space = i;
        }
    }

    //если не закрыты кавычки
    if (flag_double_quot) {
        printf("%s\b: incorrect use of quotation marks\n", str_input.substr(1, str_input.length()).c_str());
        return vector<string>();  // возвращаем пустой вектор
    }

    return args;
}

int changeDir(vector<string> args, string cur_dir, string home_dir) {
    if (args.size() > 1) {
        // Переходим в новую директорию
        if (chdir(args[1].c_str()) == 0)
            cur_dir = args[1];  // в случае успеха меняем текущую директорию
        else
            printf("cd: no such directory: %s\n", args[1].c_str());  // сообщение об ошибке
    } else if (chdir(home_dir.c_str()) != 0)  // переходим в домашнюю директорию после команды "cd"
        perror("chdir() error");

    return 0;
}

string getHostName() {
    char host_name[PATH_MAX];
    if (gethostname(host_name, sizeof(host_name)) == 0)
        return string(host_name);
    else
        return "";
}
