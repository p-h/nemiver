void foo (int a_in, char *a_str)
{
    int the_int = 0;
    the_int = *a_str;
    the_int += a_in;
}

int
main ()
{
    char *null_str = 0;
    foo (1, null_str);//must core.
    return 0;
}

