//Author: Dodji Seketeli
/*
 *This file is part of the Nemiver project
 *
 *Nemiver is free software; you can redistribute
 *it and/or modify it under the terms of
 *the GNU General Public License as published by the
 *Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Nemiver is distributed in the hope that it will
 *be useful, but WITHOUT ANY WARRANTY;
 *without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *See the GNU General Public License for more details.
 *
 *You should have received a copy of the
 *GNU General Public License along with Nemiver;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#ifndef __NMV_VAR_H__
#define __NMV_VAR_H__

#include "common/nmv-ustring.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class Var;
typedef SafePtr<Var, ObjectRef, ObjectUnref> VarSafePtr;
class Var : public Object {
    list<VarSafePtr> m_members;
    UString m_name;
    UString m_value;
    UString m_type;
    Var *m_parent;
    //if this variable is a pointer,
    //it can be dereferenced. The variable
    //it points to is stored in m_dereferenced
    VarSafePtr m_dereferenced;

public:

    Var (const UString &a_name,
         const UString &a_value,
         const UString &a_type) :
        m_name (a_name),
        m_value (a_value),
        m_type (a_type),
        m_parent (0)

    {
    }

    Var (const UString &a_name) :
        m_name (a_name),
        m_parent (0)
    {}

    Var () :
        m_parent (0)
    {}

    const list<VarSafePtr>& members () const {return m_members;}

    void append (const VarSafePtr &a_var)
    {
        if (!a_var) {return;}
        m_members.push_back (a_var);
        a_var->parent (this);
    }

    const UString& name () const {return m_name;}
    void name (const UString &a_name) {m_name = a_name;}

    const UString& value () const {return m_value;}
    void value (const UString &a_value) {m_value = a_value;}

    const UString& type () const {return m_type;}
    void type (const UString &a_type) {m_type = a_type;}

    Var* parent () const {return m_parent;}
    void parent (Var *a_parent)
    {
        m_parent = a_parent;
    }

    void to_string (UString &a_str,
                    bool a_show_var_name = false,
                    const UString &a_indent_str="") const
    {
        if (a_show_var_name) {
            if (name () != "") {
                a_str += a_indent_str + name ();
            }
        }
        if (value () != "") {
            if (a_show_var_name) {
                a_str += "=";
            }
            a_str += value ();
        }
        if (members ().empty ()) {
            return;
        }
        UString indent_str = a_indent_str + "  ";
        a_str += "\n" + a_indent_str + "{";
        list<VarSafePtr>::const_iterator it;
        for (it = members ().begin (); it != members ().end () ; ++it) {
            if (!(*it)) {continue;}
            a_str += "\n";
            (*it)->to_string (a_str, true, indent_str);
        }
        a_str += "\n" + a_indent_str + "}";
        a_str.chomp ();
    }
};//end class Var

class NEMIVER_API VarFragment {

    UString m_name;
    UString m_id;

public:

    VarFragment ()
    {
    }

    VarFragment (const UString &a_id, const UString &a_name) :
        m_name (a_name), m_id (a_id)
    {
    }

    const UString& get_id () const {return m_id;}
    void set_id (const UString &a_id) {m_id = a_id;}

    const UString& get_name () const {return m_name;}
    void set_name (const UString &a_name) {m_name = a_name;}

    bool operator= (const VarFragment &a_other)
    {
        return a_other.m_id == m_id;
    }
};//end VarFragment

NEMIVER_END_NAMESPACE (nemiver)
#endif //__NMV_VAR_H__

