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

#define BUF_SIZE 1024

#define OK 0
#define ERROR 1

using namespace std;

int changeDir(string s, int len_s, char *cur_dir, string home_dir);

int main() {
    printf("%sStart terminal...%s\n", ORANGE, RESET);

    string home_dir = getenv("HOME");  // Получаем путь к домашней директории

    if ((home_dir.c_str() == nullptr) || (chdir(home_dir.c_str()) != 0)) perror("chdir() error");

    char cur_dir[PATH_MAX];  // Буфер для хранения пути

    while (true) {
        if (getcwd(cur_dir, sizeof(cur_dir)) != nullptr)
            printf("%s%s%s: ", GREEN, cur_dir, RESET);
        else {
            perror("getcwd() error");
            break;
        }

        string s = "";

        getline(cin, s);
        int len_s = strlen(s.c_str());
        if (len_s == 0) continue;

        string command = strtok((char *)s.c_str(), " ");

        if (command == "exit") {
            break;
        } else if (command == "cd") {
            changeDir(s, len_s, cur_dir, home_dir);
            // if (len_s > 2) {
            //     string path = s.substr(strlen(command.c_str()) + 1, len_s);
            //     if (chdir(path.c_str()) == 0) {
            //         strcpy(cur_dir, path.c_str());
            //     } else {
            //         printf("Такой директории нет\n");
            //     }
            // } else {
            //     if ((chdir(home_dir.c_str()) != 0)) {
            //         perror("chdir() error");
            //     } else {
            //         cout << "Success." << endl;
            //     }
            // }

        } else {
            system(s.c_str());
        }
    }
    printf("%sClose terminal...%s\n", RED, RESET);
    return 0;
}

int changeDir(string s, int len_s, char *cur_dir, string home_dir) {
    if (len_s > 2) {
		printf("<%ld>\n", strlen("cd"));
        string path = s.substr(strlen("cd") + 1, len_s);
        if (chdir(path.c_str()) == 0)
            strcpy(cur_dir, path.c_str());
        else
            printf("cd: no such directory: %s\n", path.c_str());
    } else if (chdir(home_dir.c_str()) != 0)
        perror("chdir() error");
	
	return 0;
}
