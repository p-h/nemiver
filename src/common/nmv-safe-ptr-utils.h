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
#ifndef __NMV_SAFE_PTR_UTILS_H__
#define __NMV_SAFE_PTR_UTILS_H__

#include <glib.h>
#include "nmv-object.h"
#include "nmv-safe-ptr.h"

namespace nemiver {

namespace common {

//*******************************
//misc functors
//*******************************

struct ObjectRef {

    void
    operator () (nemiver::common::Object* a_ptr)
    {
        if (a_ptr)
            a_ptr->ref () ;
    }
};//end ObjectRef

struct ObjectUnref {

    bool
    operator () (nemiver::common::Object* a_ptr)
    {
        if (a_ptr) {
            return a_ptr->unref () ;
        }
        return true ;
    }
};//end ObjectUnRef

struct GCharRef {
    void
    operator () (char* a_ptr)
    {}
}
;

struct GCharUnref {
    bool
    operator () (char* a_ptr)
    {
        if (a_ptr) {
            g_free (a_ptr) ;
            return true ;
        }
        return true ;
    }
};

struct CharArrayRef {
    void operator () (char *a_tab)
    {
    }
};//end struct CharArraryRef

struct CharArrayUnref {
    void operator () (char *a_tab)
    {
        delete [] a_tab ;
    }
};//end struct CharArraryUnref

typedef SafePtr <gchar, GCharRef, GCharUnref> GCharSafePtr ;
typedef SafePtr <Object, ObjectRef, ObjectUnref> ObjectSafePtr ;

}//end namespace common
}//end namespace nemiver
#endif //__VERISSIMUS_SAFE_GOBJECT_PTR_H__

