#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string>
using namespace std;

/* Some Detailed Explanation:
 * 1. Some bash color code is used (such as "e[1;31m") to clearly record all detailed log
 * 2. Tested in distro "KDE Neon", using "bash" as command interpreter and "Konsole" as CLI for display
 * */

int main(int argc, char * argv[])
{
    cout << "[\e[1;32m Success \e[1;37m] Successfully started mmv.out in process \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;

    const char * read_name, * write_name;
    if(argc > 2) //toggle if argv[1], argv[2] not empty
    {
        cout << "[\e[1;32m   log   \e[1;37m] Passed-in argv detected, use user-defined path \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
        read_name = argv[1];
        write_name = argv[2];
    }
    else // use defualt mode
    {
        cout << "[\e[1;32m   log   \e[1;37m] No passed-in argv, use default files \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
        read_name = "./blake.txt";
        write_name = "./happy.tmp";
    }

    cout << "[\e[1;33m Process \e[1;37m] Trying to open file \"" << read_name << "\" \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
    int read_fd = open(read_name, O_RDWR); // open file, store the file descriptor in 'fd'
    cout << "[\e[1;33m Process \e[1;37m] Trying to create file \"" << write_name << "\" \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
    int write_fd = creat(write_name, 0644); // 2nd parameter is the permission code

    if(read_fd == -1)
    {
        cout << "[\e[1;31m  Error  \e[1;37m] Failed to open file \"" << read_name << "\"\e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
        exit(EXIT_FAILURE);
    }
    else // read file is found and opened
    {
        cout << "[\e[1;32m Success \e[1;37m] Successfully opened file \"" << read_name << "\" \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
        if(write_fd == -1)
        {
            cout << "[\e[1;31m  Error  \e[1;37m] Failed to create file \"" << write_name << "\"\e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            cout << "[\e[1;32m Success \e[1;37m] Successfully created file \"" << write_name << "\" \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;

            // all files set! read and write process starts...
            char buf[300];
            cout << "[\e[1;32m   log   \e[1;37m] Reading file content into buffer \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
            ssize_t size = read(read_fd, buf,sizeof(buf)); // it returns the total bytes read from the file
            buf[size] = '\0'; // null terminated

            cout << "[\e[1;32m   log   \e[1;37m] Adding new a line into buffer \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
            string updated = buf;
            updated = "\\\\ ---- Say Hello to s1103309! ----\\\\\n" + updated;

            cout << "[\e[1;32m   log   \e[1;37m] Writing buffer into file \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
            ssize_t wr_size = write(write_fd, updated.c_str(), updated.size());
            if(wr_size == -1)
            {
                cout << "[\e[1;31m  Error  \e[1;37m] Failed to write to file \"" << write_name << "\"\e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
                exit(EXIT_FAILURE);
            }
            else
            {
                cout << "[\e[1;32m Success \e[1;37m] Successfully writen data into file \"" << write_name << "\" \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
                exit(EXIT_SUCCESS);
            }
        }
    }

}
