#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std;
const int TARGET_VALUE = 10;

// Logging function
void log_message(const string &role, const string &message, int err_no = 0)
{
    auto now = chrono::system_clock::now();
    auto now_c = chrono::system_clock::to_time_t(now);
    auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Use stringstream for cleaner formatting
    stringstream ss;
    ss << "[" << put_time(localtime(&now_c), "%Y-%m-%d %H:%M:%S")
       << "." << setfill('0') << setw(3) << ms.count() << "]"
       << "[PID: " << getpid() << "]"
       << "[" << setfill(' ')<< left << setw(9) << role << "] " << message;

    if (err_no != 0)
    {
        ss << " (Error: " << strerror(err_no) << ")";
    }

    cerr << ss.str() << endl;
}

int main()
{
    int pipe1_fd[2];
    int pipe2_fd[2];

    // Create pipes BEFORE forking
    if (pipe(pipe1_fd) == -1)
    {
        log_message("System", "Error creating pipe 1", errno);
        return EXIT_FAILURE;
    }

    if (pipe(pipe2_fd) == -1)
    {
        log_message("System", "Error creating pipe 2", errno);
        close(pipe1_fd[0]);
        close(pipe1_fd[1]);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        // Fork failed
        log_message("System", "Error forking process", errno);
        close(pipe1_fd[0]);
        close(pipe1_fd[1]);
        close(pipe2_fd[0]);
        close(pipe2_fd[1]);
        return EXIT_FAILURE;
    }
    else if (pid == 0)
    {
        // --- Child process ---
        log_message("Receiver", "Process started.");

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
                    log_message("Receiver", "Pipe 1 closed (EOF).");
                else
                    log_message("Receiver", "Read failed on pipe 1", errno);
                break;
            }
            log_message("Receiver", "Received value: " + to_string(current_value));

            if (current_value >= TARGET_VALUE)
            {
                log_message("Receiver", "Target value reached or exceeded. Terminating.");
                break;
            }

            current_value++;

            log_message("Receiver", "Sending value: " + to_string(current_value));
            bytes_op = write(pipe2_fd[1], &current_value, sizeof(current_value));
            if (bytes_op <= 0)
            {
                log_message("Receiver", "Write failed on pipe 2", errno);
                break;
            }
        }
        
        log_message("Receiver", "Closing pipes.");
        close(pipe1_fd[0]);
        close(pipe2_fd[1]);
        log_message("Receiver", "Process finished.");
        return EXIT_SUCCESS;
    }
    else
    {
        // --- Parent process ---
        log_message("Initiator", "Process started. Child PID: " + to_string(pid));
        close(pipe1_fd[0]);
        close(pipe2_fd[1]);

        int current_value = 0;
        ssize_t bytes_op;

        log_message("Initiator", "Sending initial value: " + to_string(current_value));
        bytes_op = write(pipe1_fd[1], &current_value, sizeof(current_value));
        if (bytes_op <= 0)
        {
            log_message("Initiator", "Initial write failed on pipe 1", errno);
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
                        log_message("Initiator", "Pipe 2 closed (EOF).");
                    else
                        log_message("Initiator", "Read failed on pipe 2", errno);
                    break;
                }
                log_message("Initiator", "Received value: " + to_string(current_value));

                // Check if target reached after receive
                if (current_value >= TARGET_VALUE)
                {
                    log_message("Initiator", "Target value reached or exceeded. Informing receiver.");
                    // Send the final value back so child loop also terminates correctly
                    bytes_op = write(pipe1_fd[1], &current_value, sizeof(current_value));
                    if (bytes_op <= 0)
                    log_message("Initiator", "Final write signal failed", errno);
                    break;
                }

                current_value++;

                log_message("Initiator", "Sending value: " + to_string(current_value));
                bytes_op = write(pipe1_fd[1], &current_value, sizeof(current_value));
                if (bytes_op <= 0)
                {
                    log_message("Initiator", "Write failed on pipe 1", errno);
                    break;
                }
            }
        }

        int status;
        log_message("Initiator", "Waiting for child process to terminate...");
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            log_message("Initiator", "Child process exited with status: " + to_string(WEXITSTATUS(status)));
        }
        else
        {
            log_message("Initiator", "Child process terminated abnormally.");
        }

        log_message("Initiator", "Closing pipes.");
        close(pipe1_fd[1]);
        close(pipe2_fd[0]);

        log_message("Initiator", "Process finished.");
        return EXIT_SUCCESS;
    }
}