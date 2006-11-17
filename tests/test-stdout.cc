#include <iostream>
#include <glibmm.h>
#include "nmv-initializer.h"
#include "nmv-log-stream-utils.h"

int
main ()
{
    nemiver::common::Initializer::do_init () ;

    int i=10000;
    while (--i) {
        LOG ("line number '" << (int) i << "'") ;
    }
    return 0 ;
}

