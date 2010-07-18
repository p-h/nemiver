#include <unistd.h>
#include <iostream>

int
main ()
{
    pid_t child_pid = 0;

    child_pid = fork ();
    if (child_pid == -1) {
        // There was an error
        std::cerr << "Got an error\n";
        return -1;
    } else if (child_pid != 0) {
        // we are in the parent
        std::cout << "I forked a child OK\n";
        return 0;
    } else {
        // we are in the child
        std::cout << "I was forked by my parent. About to exec now" << std::endl;
	char *const argv[] = {NULL};
        execv ("./forkchild", argv);
    }
    return 0;
}
