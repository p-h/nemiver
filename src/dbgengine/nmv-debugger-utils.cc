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

#include "nmv-debugger-utils.h"
#include "common/nmv-exception.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (debugger_utils)

void
gen_white_spaces (int a_nb_ws,
                  std::string &a_ws_str)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    for (int i = 0; i < a_nb_ws; i++) {
        a_ws_str += ' ';
    }
}

void
dump_variable_value (IDebugger::VariableSafePtr a_var,
                     int a_indent_num,
                     std::ostringstream &a_os,
                     bool a_print_var_name)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    THROW_IF_FAIL (a_var);

    std::string ws_string;

    if (a_indent_num)
        gen_white_spaces (a_indent_num, ws_string);

    if (a_print_var_name)
        a_os << ws_string << a_var->name ();

    if (!a_var->members ().empty ()) {
        a_os << "\n"  << ws_string << "{";
        IDebugger::VariableList::const_iterator it;
        for (it = a_var->members ().begin ();
             it != a_var->members ().end ();
             ++it) {
            a_os << "\n";
            dump_variable_value (*it, a_indent_num + 2, a_os, true);
        }
        a_os << "\n" << ws_string <<  "}";
    } else {
        if (a_print_var_name)
            a_os << " = ";
        a_os << a_var->value ();
    }
}

void
dump_variable_value (IDebugger::VariableSafePtr a_var,
                     int a_indent_num,
                     std::string &a_out_str)
{
    std::ostringstream os;
    dump_variable_value (a_var, a_indent_num, os);
    a_out_str = os.str ();
}

NEMIVER_END_NAMESPACE (debugger_utils)
NEMIVER_END_NAMESPACE (nemiver)
