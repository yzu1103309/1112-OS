#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
using namespace std;

/* Some Detailed Explanation:
 * 1. Some bash color code is used (such as "\e[1;31m") to clearly record all detailed log
 * 2. wait() is used to wait for the child process to complete before parent continues
 * 3. Tested in distro "KDE Neon", using "bash" as command interpreter and "Konsole" as CLI for display
 * */

// define the flags of error, process, success, and so on.
// we'll print these later to indicate the status of the steps
#define ERROR_FLAG "[\e[1;31m  Error  \e[1;37m] "
#define PROCESS_FLAG "[\e[1;33m Process \e[1;37m] "
#define SUCCESS_FLAG "[\e[1;32m Success \e[1;37m] "
#define LOG_FLAG "[\e[1;32m   log   \e[1;37m] "
#define CLOSE_FLAG "[\e[1;33m  Close  \e[1;37m] "

int main(int argc, char * argv[])
{
    string PRINT_PID = "\e[1;35m(#" + to_string(getpid()) + ")\e[1;37m ";

    //create an array of file descriptors with size of 2
    int pfd[2];
    //the pipe is a FIFO container in the kernel, we create paths to access it
    //pfd[0] connects to the read end of pipe, while pfd[1] connects the write end
    int pipe_status = pipe(pfd);

    if(pipe_status == -1) // in case pipe error occurs
    {
        cerr << ERROR_FLAG << PRINT_PID << "Cannot connect process to the pipe" << endl;
        exit(EXIT_FAILURE);
    }
    cout << SUCCESS_FLAG << PRINT_PID << "Successfully connected process to the pipe" << endl;

    // because we connect to the pipe before fork(),
    // the paths to the pipe are shared between parent process and child pocess
    pid_t new_pID = fork(); // will return the created process ID, store in new_pID

    if(new_pID == -1) // Failed to create new process
    {
        cerr << ERROR_FLAG << "Cannot create child process" << endl;
        exit(EXIT_FAILURE);
    }
    else if(new_pID == 0) //when new_pID is 0 -> The child process (because no new process is created in child)
    {
        PRINT_PID = "\e[1;34m(#" + to_string(getpid()) + ")\e[1;37m ";

        cout << SUCCESS_FLAG << PRINT_PID << "Child process is created and running" << endl;
        cout << PROCESS_FLAG << PRINT_PID <<  "Replacing current process image with mmv.out" << endl;

        // prepare the args to pass into mmv.out
        string write_name = "./happy.tmp";
        if(argc > 2)
            write_name = argv[2];

        // STDIN_FILENO is a shared file descriptor (0) to access the input buffer
        // In child process, make STDIN_FILENO point to the read end of pipe.
        // Meaning that the pipe acts as the buffer of stdin in the child process
        // so that we're allow to share the pipe between programs using STDIN_FILENO.
        dup2(pfd[0], STDIN_FILENO);

        // replace current process image with mmv.out,
        // passing write_file name as argv.
        int status = execlp("./mmv.out", write_name.c_str(), NULL);

        if(status == -1) // when exec error occurs
        {
            cout << ERROR_FLAG << PRINT_PID << "Failed to replace process image by exec" << endl;
            exit(EXIT_FAILURE);
        }
    }
    else //when new_pID > 0 -> the parent process (new_pID stores the chid process ID)
    {
        cout << SUCCESS_FLAG << PRINT_PID << "Parent process is running" << endl;

        const char * read_name;
        //read file in the parent process
        if(argc > 2) //toggle if argv[1] and argv[2] not empty
        {
            read_name = argv[1];
            cout << LOG_FLAG << PRINT_PID << "* argv detected, use user-defined files *" << endl;
        }
        else
        {
            read_name = "./blake.txt";
            cout << LOG_FLAG << PRINT_PID << "* No argv, use the default files *" << endl;
        }

        cout << PROCESS_FLAG << PRINT_PID << "Trying to open file \"" << read_name << "\"" << endl;
        int read_fd = open(read_name, O_RDWR); // open file, store the file descriptor in 'read_fd'

        if(read_fd == -1)
        {
            cout  << ERROR_FLAG << PRINT_PID << "Failed to open file \"" << read_name << "\"" << endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            cout << SUCCESS_FLAG << PRINT_PID << "Successfully opened file \"" << read_name << "\"" << endl;

            char buf[300];
            cout << LOG_FLAG << PRINT_PID << "Reading file content into buffer" << endl;
            ssize_t rd_size = read(read_fd, buf,sizeof(buf));
            close(read_fd); // close the read file
            buf[rd_size] = '\0'; // null terminated, prevent extra characters in buf

            cout << PROCESS_FLAG << PRINT_PID << "Trying to write buffer into the pipe" << endl;

            // write buffer into the pipe
            ssize_t wr_size = write(pfd[1], buf, sizeof(buf));

            if(wr_size == -1)
            {
                cout << ERROR_FLAG << PRINT_PID << "Failed to write data into the pipe!" << endl;
                exit(EXIT_FAILURE);
            }
            else
            {
                cout << SUCCESS_FLAG << PRINT_PID << "Successfully writen data into pipe" << endl;
                // close the entrance to the write end of pipe, since we don't need it anymore
                // if we don't close it, the program might hang to wait for new content.
                close(pfd[1]);
            }
        }

        int status;
        // it waits the child process to finish.
        waitpid(new_pID, &status, 0);

        if(status == 0) // if error occurred in child process.
            cout << CLOSE_FLAG << "\e[1;34m(#" << new_pID << ")\e[1;37m " << "Child process closed with exit code 0" << endl;
        else
            cout << ERROR_FLAG << "\e[1;34m(#" << new_pID << ")\e[1;37m " << "Child process shutted down unexpectedly" << endl;

        cout << CLOSE_FLAG << PRINT_PID << "Parent process closed" << endl;
    }
}
