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
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#ifndef __NEMIVER_RUN_PROGRAM_DIALOG_H__
#define __NEMIVER_RUN_PROGRAM_DIALOG_H__

#include "nmv-object.h"
#include "nmv-safe-ptr-utils.h"

namespace nemiver {

namespace common {
class UString ;
}

using nemiver::common::UString ;
using nemiver::common::SafePtr ;

class RunProgramDialog : public common::Object {
    struct Priv ;
    SafePtr<Priv> m_priv ;

    //non copyable
    RunProgramDialog (const RunProgramDialog&) ;
    RunProgramDialog& operator= (const RunProgramDialog&) ;

    //force to create on the stack
    void* operator new (size_t) ;

    RunProgramDialog () ;
public:

    RunProgramDialog (const UString &a_resource_root_path) ;

    virtual ~RunProgramDialog () ;

    gint run () ;

    UString program_name () const ;
    void program_name (const UString &a_name) ;

    UString arguments () const ;
    void arguments (const UString &a_args) ;

    UString working_directory () const ;
    void working_directory (const UString &) ;

};//end class nemiver

}//end namespace nemiver

#endif //__NEMIVER_RUN_PROGRAM_DIALOG_H__

