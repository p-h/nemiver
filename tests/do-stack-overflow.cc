#include <cstdlib>

void
overflow_after_n_recursions (unsigned int n)
{
    if (n == 0)
        abort ();
    overflow_after_n_recursions (--n);
}

int
main (int argc, char **argv)
{
    int DEFAULT_FRAME_NUMBERS = 10000, nb_frames = 0;
    if (argc == 2) {
        nb_frames = atoi (argv[1]);
        if (nb_frames == 0)
            nb_frames = DEFAULT_FRAME_NUMBERS;
    }
    overflow_after_n_recursions (nb_frames);
}
