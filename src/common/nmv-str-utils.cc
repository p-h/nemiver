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
#include <cstring>
#include "nmv-str-utils.h"
#include "nmv-safe-ptr-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (str_utils)

using nemiver::common::UString;
using namespace nemiver::common;

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

size_t
hexa_to_int (const string &a_hexa_str)
{
    return strtoll (a_hexa_str.c_str (), NULL, 16);
}

std::string
int_to_string (size_t an_int)
{
    std::string str;
    ostringstream os;
    os << an_int;
    str = os.str ();
    return str;
}

std::vector<UString>
split (const UString &a_string, const UString &a_delim)
{
    vector<UString> result;
    if (a_string.size () == Glib::ustring::size_type (0)) {return result;}

    gint len = a_string.bytes () + 1;
    CharSafePtr buf (new gchar[len]);
    memset (buf.get (), 0, len);
    memcpy (buf.get (), a_string.c_str (), a_string.bytes ());

    gchar **splited = g_strsplit (buf.get (), a_delim.c_str (), -1);
    try {
        for (gchar **cur = splited; cur && *cur; ++cur) {
            result.push_back (UString (*cur));
        }
    } catch (...) {
    }

    if (splited) {
        g_strfreev (splited);
    }
    return result;
}

vector<UString>
split_set (const UString &a_string, const UString &a_delim_set)
{
    vector<UString> result;
    if (a_string.size () == Glib::ustring::size_type (0)) {return result;}

    gint len = a_string.bytes () + 1;
    CharSafePtr buf (new gchar[len]);
    memset (buf.get (), 0, len);
    memcpy (buf.get (), a_string.c_str (), a_string.bytes ());

    gchar **splited = g_strsplit_set (buf.get (), a_delim_set.c_str (), -1);
    try {
        for (gchar **cur = splited; cur && *cur; ++cur) {
            result.push_back (UString (*cur));
        }
    } catch (...) {
    }

    if (splited) {
        g_strfreev (splited);
    }
    return result;
}

UString
join (const vector<UString> &a_elements, const UString &a_delim)
{
    if (!a_elements.size ()) {
        return UString ("");
    }
    vector<UString>::const_iterator from = a_elements.begin ();
    vector<UString>::const_iterator to = a_elements.end ();
    return join (from, to, a_delim);
}

UString
join (vector<UString>::const_iterator &a_from,
      vector<UString>::const_iterator &a_to,
      const UString &a_delim)
{
    if (a_from == a_to) {return UString ("");}

    vector<UString>::const_iterator iter = a_from;
    UString result = *iter;
    for (; ++iter != a_to; ) {
        result += a_delim + *iter;
    }
    return result;
}

void
chomp (UString &a_string)
{
    if (!a_string.size ()) {return;}

    Glib::ustring::size_type i = 0;

    //remove the ws from the beginning of the string.
    while (!a_string.empty () && isspace (a_string.at (0))) {
        a_string.erase (0, 1);
    }

    //remove the ws from the end of the string.
    i = a_string.size ();
    if (!i) {return;}
    --i;
    while (i > 0 && isspace (a_string.at (i))) {
        a_string.erase (i, 1);
        i = a_string.size ();
        if (!i) {return;}
        --i;
    }
    if (i == 0 && isspace (a_string.at (i))) {a_string.erase (0, 1);}
}

UString::size_type
get_number_of_lines (const UString &a_string)
{
    UString::size_type res = 0;
    for (UString::const_iterator it = a_string.begin ();
         it != a_string.end () ; ++it) {
        if (*it == '\n') {++res;}
    }
    return res;
}

UString::size_type
get_number_of_words (const UString &a_string)
{
    UString::size_type i=0, num_words=0;

skip_blanks:
    for (;i < a_string.raw ().size (); ++i) {
        if (!isblank (a_string.raw ()[i]))
            goto eat_word;
    }
    goto out;

eat_word:
    num_words++;
    for (; i < a_string.raw ().size (); ++i) {
        if (isblank (a_string.raw ()[i]))
            goto skip_blanks;
    }

out:
    return num_words;
}

UString
printf (const UString &a_format, ...)
{
    UString result;
    va_list args;
    va_start (args, a_format);
    result = vprintf (a_format, args);
    va_end (args);
    return result;;
}

UString
vprintf (const UString &a_format, va_list a_args)
{
    UString result;
    GCharSafePtr str (g_strdup_vprintf (a_format.c_str (), a_args));
    result.assign (str.get ());
    return result;
}

NEMIVER_END_NAMESPACE (str_utils)
NEMIVER_END_NAMESPACE (nemiver)
