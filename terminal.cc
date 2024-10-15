#include <errno.h>
#include <limits.h>  // Для PATH_MAX
#include <setjmp.h>
#include <sys/wait.h>
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

// Структура для хранения информации о процессе
struct ProcessInfo {
    int process_number;
    pid_t pid;
    string process_name;
};

vector<char *> splitStr(string &str_input);  // Разбиение строки на аргументы
int changeDir(vector<char *> args, string home_dir);  // Смена текущей директории
string getHostName();                                 // Получение имени хоста
void sigchld_handler(int signo);                      // Обработчик сигнала
string getProcessStatus(int status);                  // Получение статуса процесса
int my_exit(vector<ProcessInfo> &processes, vector<char *> &args);  // exit
void my_ps(vector<ProcessInfo> processes);  // ps для фоновых процессов
void newProcess(vector<ProcessInfo> &processes, int &cnt_process, vector<char *> &args,
                string &str_input);  // создание нового процесса
int printPath();                     // Печать текущего пути
void deleteTerminatedProcess(int process_number, int status);  // Удаление информации о завершенном процессе

sigjmp_buf env;             // Буфер для хранения состояния программы
bool g_should_jump = true;  // Необходимо ли вернуться на ввод строки

string home_dir = getenv("HOME");  // Получаем путь к домашней директории
string cur_user = getenv("USER");  // Получаем имя пользователя
string host_name = getHostName();  // Получаем имя системы
vector<ProcessInfo> processes;     // Вектор процессов

int main() {
    // Инициализируем обработчик сигнала завершения процесса
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;  // Указание функции обработчика
    sigemptyset(&sa.sa_mask);         // Инициализация маски сигналов
    sa.sa_flags = SA_RESTART /*| SA_NOCLDSTOP*/;  // Флаги: не перезапускать, не ловить остановленные процессы
    sigaction(SIGCHLD, &sa, nullptr);  // Регистрация обработчика

    printf("%sStart terminal...%s\n", ORANGE, RESET);

    // Переходим в домашнюю директорию
    if ((home_dir.c_str() == nullptr) || (chdir(home_dir.c_str()) != 0)) perror("chdir() error");

    int cnt_process = 0;

    while (true) {
        string str_input;  // строка, вводимая пользователем

        // Если получаем сигнал о завершении процесса,
        if (sigsetjmp(env, 1) == 0) {
            // Печатаем текущую директорию
            if (printPath() == -1) break;
            getline(cin, str_input);  // ввод строки
        }

        vector<char *> args =
            splitStr(str_input);  // Разделяем строку на отдельные аргументы и записываем в вектор

        // Если вектор пуст
        if (args.empty()) continue;  // Переходим на новую итерацию

        // Обрабатываем команды
        if (strcmp(args[0], "close") == 0)
            break;
        else if (strcmp(args[0], "cd") == 0) {
            if (args.size() > 2)
                printf("cd: too many arguments\n");
            else
                changeDir(args, home_dir);
        } else if (strcmp(args[0], "mps") == 0) {
            my_ps(processes);
        } else if (strcmp(args[0], "exit") == 0) {
            if (args.size() < 2)
                printf("Usage: exit <number>\n");
            else if (args.size() > 2)
                printf("exit: too many arguments\n");
            else
                my_exit(processes, args);
        } else
            newProcess(processes, cnt_process, args, str_input);

        g_should_jump = true;
    }
    printf("%sClose terminal...%s\n", RED, RESET);

    return 0;
}

// Создание нового процесса
void newProcess(vector<ProcessInfo> &processes, int &cnt_process, vector<char *> &args, string &str_input) {
    bool is_background = false;           // В фоне процесс или нет
    if (strcmp(args.back(), "&") == 0) {  // Проверяем, открыт ли процесс в фоне
        args.pop_back();
        is_background = true;
    }

    pid_t pid = fork();  // Создаем новый процесс

    if (pid == 0) {  // Дочерний процесс
        args.push_back(NULL);
        if (execvp(args[0], args.data()) == -1) {  // Заменяем на нужный процесс
            perror("\nexecpv");
            exit(EXIT_FAILURE);  // Завершение дочернего процесса в случае ошибки execvp
        }
    } else if (pid < 0) {  // Ошибка в fork
        perror("fork");
    } else {  // Родительский процесс
        if (is_background) {  // Если процесс запущен в фоне, то не блокируем консоль
            cnt_process += 1;
            // Создаем структуру с описанием процесса
            ProcessInfo process = {cnt_process, pid, str_input.substr(1, str_input.size() - 2)};
            processes.push_back(process);  // Добавляем структуру в вектор
            printf("%s[%d] %d%s\n", PINK, cnt_process, pid, RESET);  // Выводим информацию о процессе
        } else {
            g_should_jump = false;                 // Отключаем jump
            printf("%s%d%s\n", PINK, pid, RESET);  // Выводим информацию о процессе
            int status;                            // Статус процесса
            pid_t wpid = waitpid(pid, &status, WUNTRACED);  // Ожидаем завершения процесса
            if (wpid == -1) perror("waitpid");

            printf("%s%d %s%s\n", PINK, pid, getProcessStatus(status).c_str(), RESET);
            g_should_jump = true;  // Возвращаем jump
        }
    }
}

// Разбиение строки на аргументы
vector<char *> splitStr(string &str_input) {
    vector<char *> args;  // Вектор аргументов

    // Убираем пробелы в начале строки
    size_t i = 0;
    while (str_input[i] == ' ') str_input.erase(i, 1);

    // Убираем пробелы в конце строки
    i = str_input.size() - 1;
    while (str_input[i] == ' ') {
        str_input.erase(i, 1);
        i -= 1;
    }
    str_input = " " + str_input + " ";  // Добавляем пробел в начало и конец для корректой работы
    if (str_input == "  ") return vector<char *>();  // Возвращаем пустой вектор

    // Удаляем лишние пробелы
    i = 0;
    while (i + 1 < str_input.size() && str_input[i + 1] == ' ') str_input.erase(i + 1, 1);

    bool flag_double_quot = false;  // Флаг открытия кавычек
    size_t ind_space = 0;           // Индекс последнего пробела

    // Разбиваем строку на отдельные аргументы
    for (size_t i = 0; i < str_input.size(); i++) {
        if (str_input[i] == '\\')
            str_input.erase(i, 1);  // Удаляем символ '\', i теперь указывает на экранируемый символ, на
                                    // следующей итерации i++ перепрыгнет этот экранируемый символ
        else if (str_input[i] == '"') {
            flag_double_quot = !flag_double_quot;
            str_input.erase(i, 1);
            i -= 1;  // После удаления символы смещаются на 1 назад, необходимо вернуться на 1 символ
                     // назад, чтобы попасть на следующий символ
        } else if ((str_input[i] == ' ') && (!flag_double_quot)) {
            if (i - ind_space > 1) {
                args.push_back(new char[i - (ind_space + 1) + 1]);
                strcpy(args.back(), str_input.substr(ind_space + 1, i - (ind_space + 1)).c_str());
            }
            ind_space = i;
        }
    }

    // Если не закрыты кавычки
    if (flag_double_quot) {
        printf("%s\b: incorrect use of quotation marks\n", str_input.substr(1, str_input.length()).c_str());
        return vector<char *>();  // Возвращаем пустой вектор
    }

    return args;
}

// Смена текущей директории
int changeDir(vector<char *> args, string home_dir) {
    if (args.size() > 1) {
        // Переходим в новую директорию
        if (chdir(args[1])) printf("cd: no such directory: %s\n", args[1]);  // Сообщение об ошибке
    } else if (chdir(home_dir.c_str()) != 0)  // Переходим в домашнюю директорию после команды "cd"
        perror("chdir() error");

    return 0;
}

// Получение хоста
string getHostName() {
    char host_name[PATH_MAX];
    if (gethostname(host_name, sizeof(host_name)) == 0)
        return string(host_name);
    else
        return "";
}

// Обработчик сигнала
void sigchld_handler(int signo) {
    (void)signo;

    int status;
    pid_t wpid;

    // Проходимся по всем дочерним процессам
    while ((wpid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (wpid == -1) perror("waitpid");
        deleteTerminatedProcess(wpid, status);
    }

    // Перенаправляем выполнение обратно в основное тело программы
    if (g_should_jump) siglongjmp(env, 1);
}

// Функция для получения текстового описания статуса процесса
string getProcessStatus(int status) {
    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);
        if (!exitCode)
            return "done";  // Процесс завершился успешно
        else
            return "exit code " + to_string(exitCode);  // Процесс завершился с ненулевым кодом
    } else if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        return "terminated by signal " + to_string(signal);  // Процесс был завершён сигналом
    } else if (WIFSTOPPED(status)) {
        int signal = WSTOPSIG(status);
        return "stopped by signal " + to_string(signal);  // Процесс был приостановлен сигналом
    } else
        return "running";  // Процесс продолжает выполнение
}

// Функция вывода информации о фоновых профессах
void my_ps(vector<ProcessInfo> processes) {
    printf("%-5s\t%-10s\t%s\n", "№", "PID", "NAME");
    for (auto process : processes)
        printf("%-5d\t%-10d\t%s\n", process.process_number, process.pid, process.process_name.c_str());
}

// Функция удаления завершенного процесса
void deleteTerminatedProcess(int pid, int status) {
    for (size_t i = 0; i < processes.size(); i++) {
        if (processes[i].pid == pid) {
            printf("\n%s[%d] %d %s%s\n", PINK, processes[i].process_number, processes[i].pid,
                   getProcessStatus(status).c_str(),
                   RESET);  // Выводим информацию о завершении процесса
            processes.erase(processes.begin() + i);  // Удаляем структуру из векора
            break;
        }
    }
}

// Функция принудительного завершения процесса
int my_exit(vector<ProcessInfo> &processes, vector<char *> &args) {
    int number;                               // Номер процесса
    if (args[1] == to_string(atoi(args[1])))  // Проверка на корректность ввода
        number = atoi(args[1]);               // Получаем номер процесса
    else {
        printf("exit: invalid argument\n");
        return -1;
    }

    bool flag = false;  // Флаг нахождения такого процесса
    int status;

    // Проходим по структурам всех процессов
    for (size_t i = 0; i < processes.size(); i++) {
        if (processes[i].process_number == number) {  // Сравниваем номер
            flag = true;
            kill(processes[i].pid, SIGTERM);        // Убиваем процесс
            waitpid(processes[i].pid, &status, 0);  // Ожидаем завершения процесса
            printf("%s[%d] %d %s%s\n", PINK, processes[i].process_number, processes[i].pid,
                   getProcessStatus(status).c_str(), RESET);
            processes.erase(processes.begin() + i);  // Удаляем структуру процесса из вектора
            break;
        }
    }
    if (!flag) printf("exit: a process with this number does not exist\n");

    return 0;
}

// Печать текущего пути
int printPath() {
    string cur_dir = filesystem::current_path();
    if (!cur_dir.empty()) {
        printf("%s%s@%s%s:", GREEN, cur_user.c_str(), host_name.c_str(), RESET);
        if (cur_dir.find(home_dir) == 0)
            printf("%s~%s%s$ ", CYAN, cur_dir.substr(home_dir.size(), cur_dir.size()).c_str(), RESET);
        else
            printf("%s%s%s$ ", CYAN, cur_dir.c_str(), RESET);
    } else {
        perror("current_path");
        return -1;
    }

    return 0;
}
