/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset: 4-*- */

/*Copyright (c) 2005-2006 Dodji Seketeli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <sstream>
#include "nmv-ustring.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-log-stream-utils.h"

using namespace std ;
namespace nemiver {
namespace common {

UString
UString::from_int (long long an_int)
{
    UString str ;
    ostringstream os ;
    os << an_int ;
    str = os.str ().c_str () ;
    return str ;
}

UString::UString ()
{}

UString::UString (const char *a_cstr, long a_len)
{
    if (!a_cstr) {
        Glib::ustring::operator= ("") ;
    } else {
        if (a_len < 0)
            Glib::ustring::operator= (a_cstr) ;
        else
            Glib::ustring::assign (a_cstr, a_len) ;
    }
}

UString::UString (const unsigned char *a_cstr, long a_len)
{
    const char* cstr = reinterpret_cast<const char*> (a_cstr) ;
    if (!cstr) {
        Glib::ustring::operator= ("") ;
    } else {
        if (a_len < 0)
            Glib::ustring::operator= (cstr) ;
        else
            Glib::ustring::assign (cstr, a_len) ;
    }
}

UString::UString (UString const &an_other_string): Glib::ustring (an_other_string)
{}

UString::UString (const Glib::ustring &an_other_string) :
        Glib::ustring (an_other_string)
{}

UString::UString (const string &an_other_string) :
    Glib::ustring (Glib::locale_to_utf8 (an_other_string.c_str ()))
{
}

UString::~UString ()
{}

UString&
UString::set (const gchar* a_buf, gulong a_len)
{
    if (!a_buf) {
        return *this ;
    }
    //truncate the length of string if it contains
    //zeros, otherwise, Glib::ustring throws an exception.
    gulong length_to_first_zero = strnlen (a_buf, a_len) ;
    gulong len = a_len ;
    if (length_to_first_zero != a_len) {
        len = length_to_first_zero ;
    }
    Glib::ustring::assign (a_buf, len) ;
    return *this ;
}

bool
UString::is_integer () const
{
    UString::value_type c (0) ;
    if (*this == "")
        return false ;

    for (UString::size_type i = 0; i < size () ; ++i) {
        c = (*this)[i] ;
        if (c < '0' && c > '9') {
            return false ;
        }
    }
    return true ;
}

UString&
UString::append_int (long long an_int)
{
    this->operator+= (from_int (an_int)) ;
    return *this ;
}

UString&
UString::assign_int (long long an_int)
{
    this->operator= (from_int (an_int)) ;
    return *this ;
}

UString&
UString::operator= (const char *a_cstr)
{
    if (!a_cstr) {
        Glib::ustring::operator= ("") ;
    } else {
        Glib::ustring::operator= (a_cstr) ;
    }
    return *this ;
}

UString&
UString::operator= (const unsigned char *a_cstr)
{
    return operator= (reinterpret_cast<const char*> (a_cstr)) ;
}

UString&
UString::operator= (UString const &a_cstr)
{
    if (this == &a_cstr)
        return *this ;
    Glib::ustring::operator= (a_cstr) ;
    return *this ;
}

bool
UString::operator! () const
{
    if (*this == ("")) {
        return true ;
    }
    return false ;
}

vector<UString>
UString::split (const UString &a_delim) const
{
    vector<UString> result ;
    if (size () == Glib::ustring::size_type (0)) {return result;}

    gint len = size () + 1 ;
    SafePtr<gchar> buf (new gchar[size () + 1]) ;
    memset (buf.get (), 0, len) ;
    memcpy (buf.get (), c_str (), size ()) ;

    gchar **splited = g_strsplit (buf.get (), a_delim.c_str (), -1) ;
    try {
        for (gchar **cur = splited ; cur && *cur; ++cur) {
            result.push_back (UString (*cur)) ;
        }
    } catch (...) {}

    if (splited) {
        g_strfreev (splited) ;
    }
    return result ;
}

UString
UString::join (const vector<UString> &a_elements,
               const UString &a_delim)
{
    if (!a_elements.size ()) {
        return UString ("") ;
    }
    vector<UString>::const_iterator from = a_elements.begin () ;
    vector<UString>::const_iterator to = a_elements.end () ;
    return join (from, to, a_delim) ;
}

UString
UString::join (vector<UString>::const_iterator &a_from,
               vector<UString>::const_iterator &a_to,
               const UString &a_delim)
{
    if (a_from == a_to) {return UString ("");}

    vector<UString>::const_iterator iter = a_from ;
    UString result = *iter ;
    for (; iter != a_to ; ++iter) {
        result += a_delim + *iter ;
    }
    return result ;
}

void
UString::chomp ()
{
    Glib::ustring::size_type i = 0, ws_start =0, ws_end = 0;

    //remove the ws from the beginning of the string.
    while (i < size () && isspace (at (i))) {
        LOG ("boucle 1, i: " << (int)i) ;
        ++ws_end;
        ++ i ;
    }
    Glib::ustring::size_type n = ws_end - ws_start ;
    if (n < 0) {n = 0;}
    LOG ("ws_end: " << (int)ws_end
         << ", ws_start: " << (int) ws_start
         << ", n: " << (int)n) ;
    erase (ws_start, n) ;
    LOG ("after first erase: '" << *this << "'") ;

    //remove the ws from the end of the string.
    i = size ()  - 1;
    ws_end = i ;
    ws_start = i ;
    while (i >= 0 && isspace (at (i))) {
        LOG ("boucle 2, i: " << (int)i) ;
        --ws_start ;
        --i ;
    }
    if (ws_start != ws_end) {++ws_start;}
    n = ws_end - ws_start ;
    LOG ("ws_end: " << (int)ws_end
         << ", ws_start: " << (int) ws_start
         << ", n: " << (int)n) ;
    erase (ws_start, n) ;
    LOG ("after second erase: '" << *this << "'") ;
}

}//end namespace common
}//end namespace nemiver
