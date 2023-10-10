#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

/* Some Detailed Explanation:
 * 1. Some bash color code is used (such as "e[1;31m") to clearly record all detailed log
 * 2. wait() is used to wait for the child process to complete before parent continues
 * 3. Tested in distro "KDE Neon", using "bash" as command interpreter and "Konsole" as CLI for display
 * */

int main(int argc, char * argv[])
{
    // fork() will create a new process, and copy the current process image to execute
    // which means after fork(), the parent and the child process is running the exact same thing
    // the only difference is the value in new_pID, so we need to use it to determine if it's the child or parent.
    pid_t new_pID = fork(); //return the created process ID, store in new_pID

    if(new_pID == -1) //Failed to create new process
    {
        cerr << "\e[1;31m[ Failed ]\e[1;37m Cannot create child process" << endl;
        exit(EXIT_FAILURE);
    }
    else if(new_pID == 0) //when new_pID is 0 -> The child process (because no new process is created in child)
    {
        cout << "[\e[1;32m Success \e[1;37m] Child process is created and running \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;

        cout << "[\e[1;33m Process \e[1;37m] Replacing current process image with mmv.out \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;

        int status = execvp("./mmv.out", argv); //replace the current process img with "./mmv.out", and pass in argvs

        if(status == -1) // failed to replace process image
        {
            cout << "[\e[1;31m  Error  \e[1;37m] Failed to replace process image by exec \e[1;34m(#" << getpid() << ")\e[1;37m" << endl;
            exit(EXIT_FAILURE);
        }
    }
    else //when new_pID > 0 -> the parent process (new_pID stores the chid process ID)
    {
        cout << "[\e[1;32m Success \e[1;37m] Parent process is running \e[1;35m(#" << getpid() << ")\e[1;37m" << endl;
        int status;

        // in parent, new_pID stores the chid process ID
        // so this command will make parent wait for child terminates before continues
        waitpid(new_pID, &status, 0);

        if(status == 0)
            cout << "[\e[1;33m  Close  \e[1;37m] Child process closed with exit code 0 \e[1;34m(#" << new_pID << ")\e[1;37m" << endl;
        else
            cout << "[\e[1;31m  Error  \e[1;37m] Child process shutted down unexpectedly \e[1;34m(#" << new_pID << ")\e[1;37m" << endl;

        cout << "[\e[1;33m  Close  \e[1;37m] Parent process closed \e[1;35m(#" << getpid() << ")\e[1;37m" << endl;
    }
}
