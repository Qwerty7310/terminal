#include <limits.h>  // Для PATH_MAX
#include <unistd.h>  // Для getcwd

#include <iostream>

int main() {
    std::string s = "1234";
    int i_s = atoi(s.c_str());
    std::string n_s = std::to_string(i_s);

    std::cout << "[" << i_s << "][" << n_s << "][" << (s == n_s) << "]\n";

    return 0;
}

/* ------------- FORK --------------- */

/*
#include <sys/wait.h>  // Для wait
#include <unistd.h>  // Для fork и exec

#include <iostream>

int main() {
    pid_t pid = fork();  // Создаём дочерний процесс

    if (pid == -1) {
        // Ошибка при создании дочернего процесса
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Это дочерний процесс
        std::cout << "Executing ls in child process..." << std::endl;
        execl("/bin/ls", "ls", NULL);  // Замещаем текущий процесс программой ls
        perror("execl failed");  // Если exec не сработал, выводим ошибку
        _exit(1);
    } else {
        // Это родительский процесс
        wait(NULL);  // Ожидаем завершения дочернего процесса
        std::cout << "Child process finished." << std::endl;
    }

    return 0;
}
*/
