// Author: Dodji Seketeli
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

#ifndef __NMV_DEBUGGER_UTILS_H__
#define __NMV_DEBUGGER_UTILS_H__

#include <sstream>
#include "nmv-i-debugger.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (debugger_utils)

/// \name Declarations of no-op callback slots
///
/// @{
void null_const_variable_slot (const IDebugger::VariableSafePtr &);

void null_const_variable_list_slot (const IDebugger::VariableList &);

void null_const_ustring_slot (const UString &);

void null_frame_vector_slot (const vector<IDebugger::Frame> &);

void null_frame_args_slot (const map<int, IDebugger::VariableList> &);

void null_default_slot ();

void null_disass_slot (const common::DisassembleInfo &,
		       const std::list<common::Asm> &);

void null_breakpoints_slot (const map<string, IDebugger::Breakpoint>&);

/// @}

void gen_white_spaces (int a_nb_ws,
                       std::string &a_ws_str);

template<class ostream_type>
void dump_variable_value (const IDebugger::Variable& a_var,
                          int a_indent_num,
                          ostream_type &a_os,
                          bool a_print_var_name);

void dump_variable_value (const IDebugger::Variable& a_var,
                          int a_indent_num,
                          std::string &a_out_str);

void dump_variable_value (const IDebugger::Variable& a_var);

template<class ostream_type>
ostream_type&
operator<< (ostream_type &a_out, const IDebugger::Variable &a_var);

IDebuggerSafePtr load_debugger_iface_with_confmgr ();

IDebugger::Variable::Format string_to_variable_format (const std::string &);

std::string variable_format_to_string (IDebugger::Variable::Format);

IDebuggerSafePtr load_debugger_iface_with_gconf ();

// Template implementations.

template<class ostream_type>
void
dump_variable_value (const IDebugger::Variable &a_var,
                     int a_indent_num,
                     ostream_type &a_os,
                     bool a_print_var_name)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    std::string ws_string;

    if (a_indent_num)
        gen_white_spaces (a_indent_num, ws_string);

    if (a_print_var_name)
        a_os << ws_string << a_var.name ();

    if (!a_var.members ().empty ()) {
        a_os << "\n"  << ws_string << "{";
        IDebugger::VariableList::const_iterator it;
        for (it = a_var.members ().begin ();
             it != a_var.members ().end ();
             ++it) {
            a_os << "\n";
            dump_variable_value (**it, a_indent_num + 2, a_os, true);
        }
        a_os << "\n" << ws_string <<  "}";
    } else {
        if (a_print_var_name)
            a_os << " = ";
        a_os << a_var.value ();
    }
}

template<class ostream_type>
ostream_type&
operator<< (ostream_type &a_out, const IDebugger::Variable &a_var)
{
    dump_variable_value (a_var, 4, a_out, /*print_var_name*/true);
    return a_out;
}

NEMIVER_END_NAMESPACE (debugger_utils)
NEMIVER_END_NAMESPACE (nemiver)

#endif // __NMV_DEBUGGER_UTILS_H__

