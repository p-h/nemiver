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
#ifndef __NMV_VARS_EDITOR_H__
#define __NMV_VARS_EDITOR_H__

#include <list>
#include <gtkmm/widget.h>
#include "nmv-object.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-i-debugger.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class IWorkbench ;

class NEMIVER_API VarsEditor : public nemiver::common::Object {
    //non copyable
    VarsEditor (const VarsEditor&) ;
    VarsEditor& operator= (const VarsEditor&) ;

    struct Priv ;
    SafePtr<Priv> m_priv ;

protected:
    VarsEditor () ;

public:

    VarsEditor (IDebuggerSafePtr &a_dbg, IWorkbench &a_wb) ;
    virtual ~VarsEditor () ;
    Gtk::Widget& widget () const ;
    void set_local_variables
                    (const std::list<IDebugger::VariableSafePtr> &a_vars) ;
    void show_local_variables_of_current_function () ;
};//end VarsEditor

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_VARS_EDITOR_H__


