// main.cpp (Modified)
#include <iostream>
#include <unistd.h> 
#include <sys/wait.h>
#include <cstdlib>

using namespace std;

int main() {
    int pipe1_fd[2];
    int pipe2_fd[2];

    // Create pipes BEFORE forking
    if (pipe(pipe1_fd) == -1) {
        perror("pipe1 failed");
        return EXIT_FAILURE;
    }

    if (pipe(pipe2_fd) == -1) {
        perror("pipe2 failed");
        close(pipe1_fd[0]); // Clean up first pipe
        close(pipe1_fd[1]);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork failed");
        close(pipe1_fd[0]);
        close(pipe1_fd[1]);
        close(pipe2_fd[0]);
        close(pipe2_fd[1]);
        return EXIT_FAILURE;
    } else if (pid == 0) {
        // --- Child process ---
        cout << "[PID: " << getpid() << "] Receiver started." << endl;
        
        close(pipe1_fd[1]);
        close(pipe2_fd[0]);
        
        int received_value = -1;
        ssize_t bytes_read = read(pipe1_fd[0], &received_value, sizeof(received_value));

        if (bytes_read == -1) {
            perror("Receiver: read failed");
        } else if (bytes_read == 0) {
            cout << "[PID: " << getpid() << "] Receiver: Pipe closed (EOF)." <<endl;
        } else if (bytes_read != sizeof(received_value)){
            cout << "[PID: " << getpid() << "] Receiver: Incomplete read." <<endl;
        }
         else {
            cout << "[PID: " << getpid() << "] Receiver: Received initial value: " << received_value <<endl;
        }

        close(pipe1_fd[0]);
        close(pipe2_fd[1]);
        cout << "[PID: " << getpid() << "] Receiver finishing." << endl;
        return EXIT_SUCCESS;
    } else {
        // --- Parent process ---
        cout << "[PID: " << getpid() << "] Initiator started. Child PID: " << pid << endl;
        
        close(pipe1_fd[0]);
        close(pipe2_fd[1]);

        int value_to_send = 0;
        cout << "[PID: " << getpid() << "] Initiator: Sending initial value: " << value_to_send << endl;
        ssize_t bytes_written = write(pipe1_fd[1], &value_to_send, sizeof(value_to_send));

        if (bytes_written == -1) {
            perror("Initiator: write failed");
        } else if (bytes_written != sizeof(value_to_send)) {
            cout << "[PID: " << getpid() << "] Initiator: Incomplete write." << endl;
        }
        
        int status;
        cout << "[PID: " << getpid() << "] Parent waiting for child..." << endl;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
             cout << "[PID: " << getpid() << "] Child exited with status: " << WEXITSTATUS(status) << endl;
        } else {
             cout << "[PID: " << getpid() << "] Child terminated abnormally." << endl;
        }

        close(pipe1_fd[1]);
        close(pipe2_fd[0]);

        cout << "[PID: " << getpid() << "] Initiator finishing." << endl;
        return EXIT_SUCCESS;
    }
}