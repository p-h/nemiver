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
#ifndef __NMV_OPTION_UTILS_H__
#define __NMV_OPTION_UTILS_H__

#include <glibmm.h>
#include "nmv-ustring.h"

using nemiver::common::UString;

namespace nemiver {
namespace options_utils {

class OptionDesc {
    UString m_long_name;
    gchar m_short_name;
    UString m_description;
    UString m_arg_description;
    enum Glib::OptionEntry::Flags m_flags;

public:
    OptionDesc () :
        m_short_name (0),
        m_flags ((Glib::OptionEntry::Flags)0)
    {}

    OptionDesc (const UString &a_long_name,
                const gchar a_short_name,
                const UString &a_description,
                const UString &a_arg_description,
                const Glib::OptionEntry::Flags a_flags) :
        m_long_name (a_long_name),
        m_short_name (a_short_name),
        m_description (a_description),
        m_arg_description (a_arg_description),
        m_flags (a_flags)
    {}

    const UString& long_name () {return m_long_name;}
    void long_name (const UString &a_in) {m_long_name = a_in;}

    gchar short_name () {return m_short_name;}
    void short_name (gchar a_in) {m_short_name = a_in;}

    const UString& description () {return m_description;}
    void description (const UString &a_in) {m_description = a_in;}

    const UString& arg_description () {return m_arg_description;}
    void arg_description (const UString &a_in) {m_arg_description = a_in;}

    Glib::OptionEntry::Flags flags () {return m_flags;}
    void flags (Glib::OptionEntry::Flags a_in) {m_flags = a_in;}
}; //end class OptionDesc

void option_desc_to_option (OptionDesc &a_desc,
                            Glib::OptionEntry &a_option);

void append_options_to_group (OptionDesc *a_descs,
                              int a_number_of_options,
                              Glib::OptionGroup &a_group);
}//end namespace nemiver
}//end namespace options_utils

#endif //__NMV_OPTION_UTILS_H__

