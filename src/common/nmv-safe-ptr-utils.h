/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset: 4-*- */

/*Copyright (c) 2005-2014 Dodji Seketeli
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
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

#include <glib-object.h>
#include "nmv-object.h"
#include "nmv-safe-ptr.h"
#include "nmv-namespace.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (common)

//*******************************
//misc functors
//*******************************

struct ObjectRef {

    void
    operator () (nemiver::common::Object* a_ptr)
    {
        if (a_ptr)
            a_ptr->ref ();
    }
};//end ObjectRef

struct ObjectUnref {

    void
    operator () (nemiver::common::Object* a_ptr)
    {
        if (a_ptr) {a_ptr->unref ();}
    }
};//end ObjectUnRef

struct GCharRef {
    void
    operator () (gchar* a_ptr) {if (a_ptr){}}
};

struct GCharUnref {
    bool
    operator () (gchar* a_ptr)
    {
        if (a_ptr) {
            g_free (a_ptr);
            return true;
        }
        return true;
    }
};

struct CharsRef {
    void operator () (gchar *a_tab) {if (a_tab){}}
};//end struct CharTabRef

struct DelCharsUnref {
    void operator () (gchar *a_tab)
    {
        delete [] a_tab;
    }
};//end struct CharTabUnref

struct UnicharsRef {
    void operator () (gunichar *a_tab) {if (a_tab) {}}
};
struct DelUnicharsUnref {
    void operator () (gunichar *a_tab)
    {
        delete [] a_tab;
    }
};

struct GErrorRef {
    void
    operator () (GError *)
    {}
};

struct GErrorUnref {
    void
    operator () (GError *a_error)
    {
        if (a_error) {
            g_error_free (a_error);
        }
    }
};

struct RefGObjectNative {
    void operator () (void *a_object)
    {
        if (a_object && G_IS_OBJECT (a_object)) {
            g_object_ref (G_OBJECT (a_object));
        }
    }
};

struct UnrefGObjectNative {
    void operator () (void *a_object)
    {
        if (a_object && G_IS_OBJECT (a_object)) {
            g_object_unref (G_OBJECT (a_object));
        }
    }
};

typedef SafePtr<gchar, CharsRef, GCharUnref> GCharSafePtr;
typedef SafePtr<Object, ObjectRef, ObjectUnref> ObjectSafePtr;
typedef SafePtr<gchar, CharsRef, DelCharsUnref> CharSafePtr;
typedef SafePtr<gunichar, UnicharsRef, DelUnicharsUnref> UnicharSafePtr;
typedef SafePtr<GError, GErrorRef, GErrorUnref> GErrorSafePtr;
typedef SafePtr<void*,
                RefGObjectNative,
                UnrefGObjectNative> NativeGObjectSafePtr;

NEMIVER_END_NAMESPACE(common)
NEMIVER_END_NAMESPACE(nemiver)

#endif
