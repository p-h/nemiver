#include <unistd.h>
#include <string>
#include <iostream>

int
main ()
{
    const char* tty_name = ttyname (0);

    std::cout << "tty name: " << tty_name << std::endl;
    std::string input;

    std::cout << "enter a string please: " << std::flush;
    std::cin >> input;
    std::cout << "Got input " << input << std::endl;

    return 0;
}

