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
#include <cstdlib>
#include "nmv-str-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (str_utils)

using nemiver::common::UString;

bool
extract_path_and_line_num_from_location (const UString &a_location,
                                         UString &a_file_path,
                                         unsigned &a_line_num)
{
    vector<UString> strs = a_location.split (":");
    if (strs.empty ())
        return false;
    a_file_path = strs[0];
    if (strs.size () > 1 && !strs[1].empty ())
        a_line_num = std::atoi (strs[1].c_str ());
    return true;
}
NEMIVER_END_NAMESPACE (str_utils)
NEMIVER_END_NAMESPACE (nemiver)
