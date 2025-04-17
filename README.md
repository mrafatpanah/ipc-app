# C++ Inter-Process Communication (IPC) Example using Pipes

This project demonstrates a simple Inter-Process Communication (IPC) scenario in C++ on Linux using `fork()` and anonymous pipes (`pipe()`). It involves two processes: an "Initiator" (parent) and a "Receiver" (child).

## Functionality

1.  The program starts and the main process forks itself, creating a child process.
2.  The Initiator (parent) process sends an initial integer value (0) to the Receiver (child) process through a dedicated pipe.
3.  The Receiver process reads the value, increments it by one, and sends it back to the Initiator through a second dedicated pipe.
4.  The Initiator process reads the incremented value, increments it by one again, and sends it back to the Receiver.
5.  This "ping-pong" exchange continues until the integer value reaches or exceeds 10.
6.  Once the value reaches 10, both processes detect this condition, log a termination message, close their respective pipe ends, and exit gracefully.
7.  The Initiator process waits for the Receiver process to terminate before finishing.
8.  All significant events (process start/end, sending/receiving values, errors) are logged to standard output with timestamps and Process IDs (PIDs).

## Requirements

* A C++ compiler supporting C++17 (like g++)
* CMake (version 3.10 or higher)
* A Linux-based operating system (tested on Ubuntu 22.04)

## Build Instructions

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/mrafatpanah/ipc-app.git
    cd ipc-app
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure the project using CMake:**
    ```bash
    cmake ..
    ```

4.  **Compile the project:**
    ```bash
    make
    ```
    This will create an executable file named `ipc_app` in the `build` directory.

## Running the Application

Navigate to the `build` directory and execute the compiled program:

```bash
./ipc_app
```