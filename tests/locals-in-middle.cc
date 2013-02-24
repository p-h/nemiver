int
main ()
{
    int a = 0;
    a += 1;
    a *= 2;

    {
        int b = a;
        b += 2;
        a = b;
    }

    int c = a;
    c++;
    return c;
}
