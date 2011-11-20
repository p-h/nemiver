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
#ifndef __NMV_CPP_TRAIT_H__
#define __NMV_CPP_TRAIT_H__

#include "config.h"
#include "nmv-i-lang-trait.h"

using nemiver::common::UString;
using nemiver::common::DynModIfaceSafePtr;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class CPPTrait : public ILangTrait {
    //non copyable
    CPPTrait (const CPPTrait &);
    CPPTrait& operator= (const CPPTrait &);

    UString m_name;

public:

    CPPTrait (DynamicModule *a_dynmod);
    ~CPPTrait ();
    const UString& get_name () const;
    bool has_pointers () const;
    bool is_type_a_pointer (const UString &a_type) const;
    bool is_variable_compound (const IDebugger::VariableSafePtr a_var) const;
};//end class CPPTrait

CPPTrait::CPPTrait (DynamicModule *a_dynmod):
    ILangTrait (a_dynmod)
{
    m_name = "cpptrait";
}

CPPTrait::~CPPTrait ()
{
}

const UString&
CPPTrait::get_name () const
{
    return m_name;
}

bool
CPPTrait::has_pointers () const
{
    return true;
}
bool
CPPTrait::is_type_a_pointer (const UString &a_type) const
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    LOG_DD ("type: '" << a_type << "'");

    UString type (a_type);
    type.chomp ();
    if (type[type.size () - 1] == '*') {
        LOG_DD ("type is a pointer");
        return true;
    }
    if (type.size () < 8) {
        LOG_DD ("type is not a pointer");
        return false;
    }
    UString::size_type i = type.size () - 7;
    if (!a_type.compare (i, 7, "* const")) {
        LOG_DD ("type is a pointer");
        return true;
    }
    LOG_DD ("type is not a pointer");
    return false;
}

bool
CPPTrait::is_variable_compound (const IDebugger::VariableSafePtr a_var) const
{
    if (a_var
        && (a_var->value () == "{...}"
            || a_var->value ().empty ()))
        return true;
    return false;
}

class CPPTraitModule : public DynamicModule {
public:

    void get_info (Info &a_info) const
    {
        const static Info s_info ("cpptraitmodule",
                                  "The C++ languaqe trait. "
                                  "Implements the ILangTrait interface",
                                  "1.0");
        a_info = s_info;
    }

    void do_init ()
    {
    }

    bool lookup_interface (const std::string &a_iface_name,
                           DynModIfaceSafePtr &a_iface)
    {
        if (a_iface_name == "ILangTrait") {
            a_iface.reset (new CPPTrait (this));
        } else {
            return false;
        }
        return true;
    }
};//end CPPTraitModule

NEMIVER_END_NAMESPACE (nemiver)

//the dynmod initial factory.
extern "C" {
bool
NEMIVER_API nemiver_common_create_dynamic_module_instance (void **a_new_instance)
{
    *a_new_instance = new nemiver::CPPTraitModule ();
    return (*a_new_instance != 0);
}
}//end extern C

#endif // __NMV_CPP_TRAIT_H__

