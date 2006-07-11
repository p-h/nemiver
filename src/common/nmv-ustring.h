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
#ifndef __NEMIVER_USTRING_H__
#define __NEMIVER_USTRING_H__

#include <vector>
#include <string>
#include "config.h"
#include <glibmm.h>
#include "nmv-api-macros.h"

using namespace std ;

namespace nemiver {
namespace common {

class NEMIVER_API UString: public Glib::ustring {

public:
    UString () ;
    UString (const char *a_cstr, long a_len=-1) ;
    UString (const unsigned char *a_cstr, long a_len=-1) ;
    UString (const Glib::ustring &an_other_string) ;
    UString (const string &an_other_string) ;
    UString (UString const &an_other_string) ;
    virtual ~UString () ;
    UString& set (const gchar* a_buf, gulong a_len) ;
    static UString from_int (long long an_int) ;
    bool is_integer () const ;
    UString& append_int (long long an_int) ;
    UString& assign_int (long long) ;
    UString& operator= (const char *a_cstr) ;
    UString& operator= (const unsigned char *a_cstr) ;
    UString& operator= (UString const &a_cstr) ;
    bool operator! () const ;
    vector<UString> split (const UString &a_delim) const ;
};//end class UString

}//end namespace common
}//end namespace nemiver

#endif

