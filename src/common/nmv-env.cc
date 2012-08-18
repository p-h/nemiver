/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4; -*- */

/*Copyright (c) 2005-2006 Dodji Seketeli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <fstream>
#include <glibmm.h>
#include "nmv-env.h"
#include "nmv-ustring.h"
#include "nmv-dynamic-module.h"

using namespace std;


NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (common)
NEMIVER_BEGIN_NAMESPACE (env)

class Initializer
{

public:
    Initializer ()
    {
        Glib::thread_init ();
    }

    ~Initializer ()
    {
    }
};

void
do_init ()
{
    static Initializer init;
}

const UString&
get_system_config_file ()
{
    static UString path;
    if (path.size () == 0) {
        vector<string> path_elements;
        path_elements.push_back (get_system_config_dir ());
        path_elements.push_back ("nemiver.conf");
        path = Glib::build_filename (path_elements).c_str ();
    }
    return path;
}

const UString&
get_data_dir ()
{
#ifndef DATADIR
#error The macro DATADIR must be set at compile time !
#endif
    static UString s_data_dir (DATADIR);
    return s_data_dir;
}
const UString&
get_system_lib_dir ()
{
#ifndef SYSTEM_LIBDIR
#error The macro SYSTEM_LIBDIR must be set at compile time !
#endif
    static UString s_system_lib_dir (SYSTEM_LIBDIR);
    return s_system_lib_dir;
}

const UString&
get_gtkbuilder_files_dir ()
{
    static UString s_gtkbuilder_files_dir;
    if (s_gtkbuilder_files_dir == "") {
        vector<string> path_elems;
        path_elems.push_back (get_data_dir ());
        path_elems.push_back ("nemiver");
        path_elems.push_back ("ui");
        s_gtkbuilder_files_dir = Glib::build_filename (path_elems).c_str ();
    }
    return s_gtkbuilder_files_dir;
}

const UString&
get_menu_files_dir ()
{
    static UString s_menu_files_dir;
    if (s_menu_files_dir == "") {
        vector<string> path_elems;
        path_elems.push_back (get_data_dir ());
        path_elems.push_back ("nemiver");
        path_elems.push_back ("menus");
        s_menu_files_dir = Glib::build_filename (path_elems).c_str ();
    }
    return s_menu_files_dir;
}

const UString&
get_image_files_dir ()
{
    static UString s_menu_files_dir;
    if (s_menu_files_dir == "") {
        vector<string> path_elems;
        path_elems.push_back (get_data_dir ());
        path_elems.push_back ("nemiver");
        path_elems.push_back ("images");
        s_menu_files_dir = Glib::build_filename (path_elems).c_str ();
    }
    return s_menu_files_dir;
}

const UString&
get_install_prefix ()
{
#ifndef NEMIVER_INSTALL_PREFIX
#error The macro NEMIVER_INSTALL_PREFIX must be set at compile time !
#endif
    //this macro must be computed at compile time.
    //maybe we'll need to change the way we compute
    //this to allow the choice of install prefix
    //at install time.
    static UString s_path (NEMIVER_INSTALL_PREFIX);
    return s_path;
}

const UString&
get_system_config_dir ()
{
#ifndef NEMIVER_SYSTEM_CONFIG_DIR
#error The macro NEMIVER_SYSTEM_CONFIG_DIR must be set at compile time !
#endif
    //this macro must be computed at compile time.
    static UString s_path (NEMIVER_SYSTEM_CONFIG_DIR);
    return s_path;
}

const UString&
get_system_modules_dir ()
{
#ifndef NEMIVER_MODULES_DIR
#error The macro NEMIVER_MODULES_DIR must be set at compile time !
#endif
    //this macro must be set at compile time.
    static UString s_path (NEMIVER_MODULES_DIR);
    return s_path;
}

const UString&
get_system_modules_config_file ()
{
    static UString path;
    if (path.size () == 0) {
        vector<string> path_elements;
        path_elements.push_back (get_system_config_dir ());
        path_elements.push_back ("nemivermodules.conf");
        path = Glib::build_filename (path_elements).c_str ();
    }
    return path;
}

const UString&
get_system_plugins_dir ()
{

#ifndef NEMIVER_PLUGINS_DIR
#error The macro NEMIVER_PLUGINS_DIR must be set at compile time !
#endif

    static UString s_path (NEMIVER_PLUGINS_DIR);
    return s_path;
}

const UString&
get_user_db_dir ()
{
    static UString s_path;
    if (s_path.size () == 0) {
        vector<string> path_elements;
        string home = Glib::get_home_dir ();
        path_elements.push_back (home);
        path_elements.push_back (".nemiver");
        s_path = Glib::build_filename (path_elements).c_str ();
    }
    return s_path;
}

const UString&
get_gdb_program ()
{

#ifndef GDB_PROG
#error the macro GDB_PROG must be set at compile time !
#endif

    static const UString s_gdb_prog (GDB_PROG);
    return s_gdb_prog;
}

bool
create_user_db_dir ()
{
    gint error =
        g_mkdir_with_parents (get_user_db_dir ().c_str (), S_IRWXU);
    if (!error)
        return true;
    return false;
}

UString
build_path_to_gtkbuilder_file (const UString &a_gtkbuilder_file_name)
{
    UString dir (get_gtkbuilder_files_dir ());
    vector<string> path_elems;
    path_elems.push_back (dir.c_str ());
    path_elems.push_back (a_gtkbuilder_file_name);
    UString path_to_gtkbuilder = Glib::build_filename (path_elems).c_str ();
    if (!Glib::file_test (path_to_gtkbuilder.c_str (),
                         Glib::FILE_TEST_IS_REGULAR)) {
        THROW ("couldn't find file " + path_to_gtkbuilder);
    }
    return path_to_gtkbuilder;
}

UString
build_path_to_menu_file (const UString &a_menu_file_name)
{
    UString dir (get_menu_files_dir ()), path_to_menu_file;
    vector<string> path_elems;
    path_elems.push_back (dir.c_str ());
    path_elems.push_back (a_menu_file_name);
    path_to_menu_file = Glib::build_filename (path_elems).c_str ();
    if (!Glib::file_test (path_to_menu_file.c_str (),
                          Glib::FILE_TEST_IS_REGULAR)) {
        THROW ("couldn't find file " + path_to_menu_file);
    }
    return path_to_menu_file;
}

UString
build_path_to_image_file (const UString &a_image_file_name)
{
    UString dir (get_image_files_dir ());
    vector<string> path_elems;
    path_elems.push_back (dir.c_str ());
    path_elems.push_back (a_image_file_name);
    UString result = Glib::build_filename (path_elems).c_str ();
    if (!Glib::file_test (result.c_str (),
                          Glib::FILE_TEST_IS_REGULAR)) {
        THROW ("couldn't find file " + result);
    }
    return result;
}

bool
build_path_to_executable (const UString &a_exe_name,
                          UString &a_path_to_exe)
{
    std::string path = Glib::find_program_in_path (a_exe_name);
    if (path.empty ())
        return false;
    a_path_to_exe = Glib::filename_to_utf8 (path);
    return true;
}

/// Find a file name, by looking in a given set of directories
/// \param a_file_name the file name to look for.
/// \param a_where_to_look the list of directories where to look for
/// \param a_absolute_file_path the absolute path of the file name
/// found. This parameter is set iff the function return true.
/// \return true upon successful completion, false otherwise.
bool
find_file (const UString &a_file_name,
           const list<UString> &a_where_to_look,
           UString &a_absolute_file_path)
{
    string file_name = Glib::filename_from_utf8 (a_file_name);
    string path, candidate;

    if (a_file_name.empty ())
        return false;

    // first check if this is an absolute path
    if (Glib::path_is_absolute (file_name)) {
        if (Glib::file_test (file_name, Glib::FILE_TEST_IS_REGULAR)) {
            a_absolute_file_path = Glib::filename_to_utf8 (file_name);
            return true;
        }
    }

    // Otherwise, lookup the file in the directories we where given
    for (list<UString>::const_iterator i = a_where_to_look.begin ();
         i != a_where_to_look.end ();
         ++i) {
        path = Glib::filename_from_utf8 (*i);
        candidate = Glib::build_filename (path, file_name);
        if (Glib::file_test (candidate, Glib::FILE_TEST_IS_REGULAR)) {
            a_absolute_file_path = Glib::filename_to_utf8 (candidate);
            return true;
        }
    }
    return false;
}

/// Given a file path P and a line number N , reads the line N from P
/// and return it iff the function returns true. This is useful
/// e.g. when forging a mixed source/assembly source view, and we want
/// to display a source line N from a file P.
///
/// \param a_file_path the absolute file path to consider
/// \param a_line_number the line number to consider
/// \param a_line the string containing the resulting line read, if
/// and only if the function returned true.
/// \return true upon successful completion, false otherwise.
bool
read_file_line (const UString &a_file_path,
                int a_line_number,
                string &a_line)
{
    if (a_file_path.empty ())
        return false;

    bool found_line = false;
    int line_num = 1;
    char c = 0;

    NEMIVER_TRY;

    std::ifstream file (a_file_path.c_str ());

    if (!file.good ()) {
        LOG_ERROR ("Could not open file " + a_file_path);
        return false;
    }

    while (true) {
        if (line_num == a_line_number) {
            found_line = true;
            break;
        }
        file.get (c);
        if (!file.good ())
            break;
        if (c == '\n')
            ++line_num;
    }
    if (found_line) {
        a_line.clear ();
        while (true) {
            file.get (c);
            if (!file.good ())
                break;
            if (c == '\n')
                break;
            a_line += c;
        }
    }
    file.close ();

    NEMIVER_CATCH_NOX;

    return found_line;
}

NEMIVER_END_NAMESPACE (env)
NEMIVER_END_NAMESPACE (common)
NEMIVER_END_NAMESPACE (nemiver)

