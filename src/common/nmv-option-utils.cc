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
#include "config.h"
#include "nmv-option-utils.h"

namespace nemiver {
namespace options_utils {

void
option_desc_to_option (OptionDesc &a_desc,
                       Glib::OptionEntry &a_entry)
{
    a_entry.set_long_name (a_desc.long_name ());
    a_entry.set_short_name (a_desc.short_name ());
    a_entry.set_description (a_desc.description ());
    a_entry.set_arg_description (a_desc.arg_description ());
    a_entry.set_flags (a_desc.flags ());
}

void
append_options_to_group (OptionDesc *a_descs,
                         int a_number_of_options,
                         Glib::OptionGroup &a_group)
{
    Glib::OptionEntry entry;

    for (int i = 0; i < a_number_of_options ; ++i) {
        option_desc_to_option (a_descs[i], entry);
        a_group.add_entry (entry);
    }
}

}//end namespace options_utils
}//end namespace nemiver

