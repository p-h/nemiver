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
#ifndef __NEMIVER_EXCEPTION_H__
#define __NEMIVER_EXCEPTION_H__

#include <stdexcept>
#include "nmv-ustring.h"
#include "nmv-log-stream-utils.h"

namespace nemiver {
namespace common {

class NEMIVER_EXCEPTION_API Exception: public std::runtime_error
{
public:
    Exception (const char* a_reason) ;
    Exception (const UString &a_reason) ;
    Exception (const Exception &a_other) ;
    Exception (const std::exception &) ;
    Exception& operator= (const Exception &a_other) ;
    virtual ~Exception () throw ();
    const char* what () const throw ();
};//class Exception

#define THROW_IF_FAIL(a_cond) \
if (!(a_cond)) { \
LOG ("condition (" << #a_cond << ") failed; raising exception\n" ) ;\
throw nemiver::common::Exception \
    (nemiver::common::UString ("Assertion failed: ") + #a_cond)  ;\
}

#define THROW_IF_FAIL2(a_cond, a_reason) \
if (!(a_cond)) { \
LOG ("condition (" << #a_cond << ") failed; raising exception " << a_reason <<"\n");\
throw nemiver::common::Exception (a_reason)  ;\
}

#define THROW_IF_FAIL3(a_cond, type, a_reason) \
if (!(a_cond)) { \
LOG ("condition (" << #a_cond << ") failed; raising exception " << #type << \
<< ":  " << a_reason << "\n" ) ; throw type (a_reason)  ;\
}

#define THROW(a_reason) \
LOG ("raised exception: "<< (nemiver::common::UString (a_reason)) << "\n"); \
throw nemiver::common::Exception (nemiver::common::UString (a_reason))  ;

#define THROW_EMPTY \
LOG ("raised empty exception " << endl) ; \
throw ;

#define THROW_EXCEPTION(type, message) \
LOG ("raised " << #type << ": "<< message<< "\n") ; \
throw type (message) ;

#define TRACE_EXCEPTION(exception) \
LOG ("catched exception: " << exception.what () << "\n")

#define RETHROW_EXCEPTION(exception) \
LOG ("catched and rethrowing exception: " << exception.what() << "\n")

#define RETURN_VAL_IF_FAIL(expression, value) \
if (!(expression)) { \
LOG ("assertion " << #expression << " failed. Returning " << #value << "\n") ; \
return value ; \
}

#define RETURN_IF_FAIL(expression) \
if (!(expression)) { \
LOG ("assertion " << #expression << " failed. Returning.\n") ; \
return ; \
}

}//end namespace common
}//end namespace nemiver 

#endif //__NEMIVER_EXCEPTION_H__
