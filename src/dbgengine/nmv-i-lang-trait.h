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
#ifndef __NMV_I_LANG_TRAIT_H__
#define __NMV_I_LANG_TRAIT_H__

#include "common/nmv-ustring.h"
#include "common/nmv-dynamic-module.h"
#include "nmv-i-debugger.h"

using nemiver::common::UString;
using nemiver::common::DynModIface;
using nemiver::common::DynamicModule;
using nemiver::common::SafePtr;
using nemiver::common::ObjectRef;
using nemiver::common::ObjectUnref;
using nemiver::common::SafePtr;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class ILangTrait;
typedef SafePtr<ILangTrait, ObjectRef, ObjectUnref> ILangTraitSafePtr;
class NEMIVER_API ILangTrait : public DynModIface {
    //non copyable
    ILangTrait (const ILangTrait &);
    ILangTrait& operator= (const ILangTrait &);

protected:
    ILangTrait (DynamicModule *a_dynmod) : DynModIface (a_dynmod)
    {
    }

public:

    virtual ~ILangTrait () {}

    virtual const UString& get_name () const = 0;

    /// \name language features tests
    /// @{
    virtual bool has_pointers () const = 0;
    /// @}

    /// \name language features
    /// @{
    virtual bool is_type_a_pointer (const UString &a_type) const = 0;
    virtual bool is_variable_compound
                        (const nemiver::IDebugger::VariableSafePtr) const = 0;
    /// @}
};//end class ILangTrait

NEMIVER_END_NAMESPACE (nemiver)

#endif // __NMV_I_LANG_TRAIT_H__
