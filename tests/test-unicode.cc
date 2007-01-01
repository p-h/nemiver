#include <iostream>
#include <string>
#include <glibmm.h>
#include "nmv-ustring.h"
#include "nmv-initializer.h"

using nemiver::common::Initializer ;
using nemiver::common::UString ;
using nemiver::common::WString ;
using nemiver::common::ustring_to_wstring ;
using nemiver::common::wstring_to_ustring ;

gunichar s_wstr[] = {230, 231, 232, 233, 234, 0} ;

int
main ()
{
    Initializer::do_init () ;
    UString utf8_str ;
    if (!wstring_to_ustring (s_wstr, utf8_str)) {
        cerr << "failed to convert WString to UString\n" ;
        cerr << "KO" << endl;
        return -1 ;
    }
    unsigned int s_wstr_len = (sizeof (s_wstr)/sizeof (gunichar))-1 ;
    if (s_wstr_len != utf8_str.size ()) {
        cerr << "failed to convert WString to UString properly\n" ;
        cerr << "KO" << endl;
        return -1 ;
    }
    cout << "converted string: '"
         << Glib::locale_from_utf8 (utf8_str) << "'"
         << endl ;
    cout << "strlen: " << utf8_str.size () <<  endl ;

    WString wstr ;
    if (!ustring_to_wstring (utf8_str, wstr)) {
        cerr << "failed to convert UString to WString\n" ;
        cerr << "KO" << endl ;
        return -1 ;
    }

    if (wstr.size () != utf8_str.size ()) {
        cerr << "failed to convert UString to WString properly: size mismatch"
             << endl;
        cerr << "KO" << endl ;
        return -1 ;
    }
    if (wstr.compare (s_wstr)) {
        cerr << "failed to convert UString to WString properly: different strs"
             << endl;
        cerr << "KO" << endl ;
        return -1 ;
    }
    cout << "OK" << endl;
    return 0 ;
}

