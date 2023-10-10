#include <iostream>
#include <unistd.h>
#include <fcntl.h>
using namespace std;

/* Some Detailed Explanation:
 * 1. Some bash color code is used (such as "e[1;31m") to clearly record all detailed log
 * 2. Tested in distro "KDE Neon", using "bash" as command interpreter and "Konsole" as CLI for display
 * */

#define ERROR_FLAG "[\e[1;31m  Error  \e[1;37m] "
#define PROCESS_FLAG "[\e[1;33m Process \e[1;37m] "
#define SUCCESS_FLAG "[\e[1;32m Success \e[1;37m] "
#define LOG_FLAG "[\e[1;32m   log   \e[1;37m] "

int main(int argc, char * argv[])
{
    string PRINT_PID = "\e[1;34m(#" + to_string(getpid()) + ")\e[1;37m ";

    cout << SUCCESS_FLAG << PRINT_PID << "Successfully started mmv.out in process" << endl;

    char * write_name = argv[0];

    cout << PROCESS_FLAG << PRINT_PID << "Trying to create file \"" << write_name << "\"" << endl;
    int write_fd = creat(write_name, 0644); // the 2nd parameter is the 'permission code'

    if(write_fd == -1)
    {
        cout << ERROR_FLAG << PRINT_PID << "Failed to create file \"" << write_name << "\"" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << SUCCESS_FLAG << PRINT_PID << "Successfully created file \"" << write_name << "\"" << endl;

        /* ------ Write file set! Read from pipe and write to file ------ */
        char buf[300];

        cout << PROCESS_FLAG << PRINT_PID << "Trying to read data from the existed pipe" << endl;
        // The pipe is now acted as the "stdin buffer"!
        // We use "cin.getline()" to read from stdin buffer (the existed pipe)
        // Because the data in the pipe is a null terminated str, we make "getline()" stop when it encounters '\0'
        cin.getline(buf, 300, '\0');

        cout << LOG_FLAG << PRINT_PID << "Adding new a line into the buffer string" << endl;
        string updated = buf;
        updated = "\\\\ ---- Say Hello to s1103309! ----\\\\\n" + updated;

        cout << PROCESS_FLAG << PRINT_PID << "Writing buffer into file" << endl;
        ssize_t wr_size = write(write_fd, updated.c_str(), updated.size());
        if(wr_size == -1) // if can't write into file
        {
            cout << ERROR_FLAG << PRINT_PID << "Failed to write to file \"" << write_name << "\"" << endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            cout << SUCCESS_FLAG << PRINT_PID << "Successfully writen " << wr_size <<" Bytes into \"" << write_name << "\"" << endl;
            exit(EXIT_SUCCESS);
        }
    }
}
