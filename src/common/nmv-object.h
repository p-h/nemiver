/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4;-*- */

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
#ifndef __NMV_OBJECT_H__
#define __NMV_OBJECT_H__

#include "nmv-api-macros.h"
#include "nmv-namespace.h"
#include "nmv-safe-ptr.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (common)

struct ObjectPriv;
class UString;

class NEMIVER_API Object {
    friend struct ObjectPriv;

protected:
    SafePtr<ObjectPriv> m_priv;

public:

    Object ();

    Object (Object const &);

    Object& operator= (Object const&);

    virtual ~Object ();

    void ref ();

    void unref ();

    void enable_refcount (bool a_enabled=true);

    bool is_refcount_enabled () const;

    long get_refcount () const;

    void attach_object (const UString &a_key,
                        const Object *a_object);

    bool get_attached_object (const UString &a_key,
                              const Object *&a_object);

};//end class Object

NEMIVER_END_NAMESPACE (common)
NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_OBJECT_H__

