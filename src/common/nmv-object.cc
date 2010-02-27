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
#include <cstdlib>
#include <map>
#include "nmv-object.h"
#include "nmv-ustring.h"

using namespace std;

namespace nemiver {
namespace common {

struct ObjectPriv {
    long refcount;
    bool refcount_enabled;
    map<UString, const Object*> objects_map;

    ObjectPriv () :
        refcount (1),
        refcount_enabled (true)
    {}
};//end struct ObjectPriv

Object::Object ():
        m_priv (new ObjectPriv ())
{
}

Object::Object (Object const &a_object):
        m_priv (new ObjectPriv ())
{
    *m_priv = *a_object.m_priv;
}

Object&
Object::operator= (Object const &a_object)
{
    if (this == &a_object)
        return *this;
    *m_priv = *a_object.m_priv;
    return *this;
}

Object::~Object ()
{
}

void
Object::ref ()
{
    if (!is_refcount_enabled ()) {return;}
    m_priv->refcount ++;
}

void
Object::unref ()
{
    if (!is_refcount_enabled ()) {return;}
    if (m_priv && m_priv->refcount) {
        m_priv->refcount --;
    }

    if (m_priv && m_priv->refcount <= 0) {
        m_priv.reset ();
        delete this;
    }
}

void
Object::enable_refcount (bool a_enabled)
{
    if (m_priv) {
        m_priv->refcount_enabled = a_enabled;
    }
}

bool
Object::is_refcount_enabled () const
{
    bool res (true);
    if (m_priv) {
        res = m_priv->refcount_enabled;
    }
    return res;
}

long
Object::get_refcount () const
{
    return m_priv->refcount;
}

void
Object::attach_object (const UString &a_key,
                       const Object *a_object)
{
    m_priv->objects_map[a_key] = a_object;
}

bool
Object::get_attached_object (const UString &a_key,
                             const Object *&a_object)
{
    map<UString, const Object*>::const_iterator it =
                                    m_priv->objects_map.find (a_key);
    if (it == m_priv->objects_map.end ()) {
        return false;
    }
    a_object = it->second;
    return true;
}

}//end namespace common
}//end namespace nemiver

