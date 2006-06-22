#include <stdio.h>

void
func1 ()
{
    int i = 0 ;
    ++i ;
}

void
func2 (int a_a, int a_b)
{
    int j = a_a ;
    ++j ;
    j = j + a_b ;
}

int
main (int a_argc, char *a_argv[])
{
    func1 () ;

    func2 (1, 2) ;

    return 0 ;
}

