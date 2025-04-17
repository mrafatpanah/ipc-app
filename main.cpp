// main.cpp (Modified)
#include <iostream>
#include <unistd.h> 
#include <sys/wait.h>
#include <cstdlib>

using namespace std;

int main() {
    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork failed");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        // --- Child process ---
        cout << "[PID: " << getpid() << "] Receiver started." << endl;
        cout << "[PID: " << getpid() << "] Receiver finishing." << endl;
        return EXIT_SUCCESS;
    } else {
        // --- Parent process ---
        cout << "[PID: " << getpid() << "] Initiator started. Child PID: " << pid << endl;
        int status;
        cout << "[PID: " << getpid() << "] Parent waiting for child..." << endl;
        waitpid(pid, &status, 0);
         if (WIFEXITED(status)) {
             cout << "[PID: " << getpid() << "] Child exited with status: " << WEXITSTATUS(status) << endl;
         } else {
             cout << "[PID: " << getpid() << "] Child terminated abnormally." << endl;
         }
        cout << "[PID: " << getpid() << "] Initiator finishing." << endl;
        return EXIT_SUCCESS;
    }
}