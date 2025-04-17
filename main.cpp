// main.cpp (Modified)
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

using namespace std;
const int TARGET_VALUE = 10;

int main()
{
    int pipe1_fd[2];
    int pipe2_fd[2];

    // Create pipes BEFORE forking
    if (pipe(pipe1_fd) == -1)
    {
        perror("pipe1 failed");
        return EXIT_FAILURE;
    }

    if (pipe(pipe2_fd) == -1)
    {
        perror("pipe2 failed");
        close(pipe1_fd[0]);
        close(pipe1_fd[1]);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        // Fork failed
        perror("fork failed");
        close(pipe1_fd[0]);
        close(pipe1_fd[1]);
        close(pipe2_fd[0]);
        close(pipe2_fd[1]);
        return EXIT_FAILURE;
    }
    else if (pid == 0)
    {
        // --- Child process ---
        cout << "[PID: " << getpid() << "] Receiver started." << endl;

        close(pipe1_fd[1]);
        close(pipe2_fd[0]);

        int current_value = 0;
        ssize_t bytes_op;
        
        // communication loop
        while (true)
        {
            bytes_op = read(pipe1_fd[0], &current_value, sizeof(current_value));
            if (bytes_op <= 0)
            {
                if (bytes_op == 0)
                    cout << "[PID: " << getpid() << "] Reciever: Pipe 1 closed." << endl;
                else
                    perror("Reciever: read failed on pipe 1");
                break;
            }
            cout << "[PID: " << getpid() << "] Reciever: Received: " << current_value << endl;

            if (current_value >= TARGET_VALUE)
            {
                cout << "[PID: " << getpid() << "] Reciever: Target reached. Terminating." << endl;
                break;
            }

            current_value++;

            cout << "[PID: " << getpid() << "] Reciever: Sending: " << current_value << endl;
            bytes_op = write(pipe2_fd[1], &current_value, sizeof(current_value));
            if (bytes_op <= 0)
            { 
                perror("Reciever: write failed on pipe 2");
                break;
            }
        }

        close(pipe1_fd[0]);
        close(pipe2_fd[1]);
        cout << "[PID: " << getpid() << "] Receiver finishing." << endl;
        return EXIT_SUCCESS;
    }
    else
    {
        // --- Parent process ---
        cout << "[PID: " << getpid() << "] Initiator started. Child PID: " << pid << endl;

        close(pipe1_fd[0]);
        close(pipe2_fd[1]);

        int current_value = 0;
        ssize_t bytes_op;

        cout << "[PID: " << getpid() << "] Initiator: Sending initial: " << current_value << endl;
        bytes_op = write(pipe1_fd[1], &current_value, sizeof(current_value));
        if (bytes_op <= 0)
        {
            perror("Initiator: initial write failed on pipe 1");
        }
        else
        {
            // Communication loop
            while (current_value < TARGET_VALUE)
            {
                bytes_op = read(pipe2_fd[0], &current_value, sizeof(current_value));
                if (bytes_op <= 0)
                {
                    if (bytes_op == 0)
                        cout << "[PID: " << getpid() << "] Initiator: Pipe 2 closed." << endl;
                    else
                        perror("Initiator: read failed on pipe 2");
                    break;
                }
                cout << "[PID: " << getpid() << "] Initiator: Received: " << current_value << endl;

                // Check if target reached after receive
                if (current_value >= TARGET_VALUE) {
                    cout << "[PID: " << getpid() << "] Initiator: Target reached by child. Signaling confirmation." << endl;
                    // Send the final value back so child loop also terminates correctly
                    bytes_op = write(pipe1_fd[1], &current_value, sizeof(current_value));
                    if (bytes_op <= 0)
                        perror("Initiator: final write signal failed");
                    break;
                }

                current_value++;

                cout << "[PID: " << getpid() << "] Initiator: Sending: " << current_value << endl;
                bytes_op = write(pipe1_fd[1], &current_value, sizeof(current_value));
                if (bytes_op <= 0)
                {
                    perror("Initiator: write failed on pipe 1");
                    break;
                }
            }
        }

        int status;
        cout << "[PID: " << getpid() << "] Parent waiting for child..." << endl;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            cout << "[PID: " << getpid() << "] Child exited with status: " << WEXITSTATUS(status) << endl;
        }
        else
        {
            cout << "[PID: " << getpid() << "] Child terminated abnormally." << endl;
        }

        close(pipe1_fd[1]);
        close(pipe2_fd[0]);

        cout << "[PID: " << getpid() << "] Initiator finishing." << endl;
        return EXIT_SUCCESS;
    }
}