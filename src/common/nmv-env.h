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
#ifndef __NEMIVER_ENV_H__
#define __NEMIVER_ENV_H__

#include <libglademm.h>
#include "nmv-api-macros.h"
#include "nmv-ustring.h"
#include "nmv-exception.h"

namespace nemiver {
namespace common {
namespace env {

void do_init () ;

NEMIVER_API const UString& get_system_config_dir () ;

NEMIVER_API const UString& get_system_config_file () ;

NEMIVER_API const UString& get_data_dir () ;

NEMIVER_API const UString& get_glade_files_dir () ;

NEMIVER_API const UString& get_menu_files_dir () ;

NEMIVER_API const UString& get_image_files_dir () ;

NEMIVER_API const UString& get_install_prefix () ;

NEMIVER_API const UString& get_system_modules_config_file () ;

NEMIVER_API const UString& get_system_modules_dir () ;

NEMIVER_API const UString& get_system_modules_config_file () ;

NEMIVER_API const UString& get_system_plugins_dir () ;

NEMIVER_API const UString& get_user_db_dir () ;

NEMIVER_API const UString& get_gdb_program () ;

NEMIVER_API bool create_user_db_dir () ;

NEMIVER_API UString build_path_to_glade_file (const UString &a_glade_file_name) ;

NEMIVER_API UString build_path_to_menu_file (const UString &a_ui_file_name) ;

NEMIVER_API UString build_path_to_image_file (const UString &a_image_file_name) ;

template <class T>
T*
get_widget_from_glade (const Glib::RefPtr<Gnome::Glade::Xml> &a_glade,
                       const UString &a_widget_name)
{
    Gtk::Widget *widget = a_glade->get_widget (a_widget_name) ;
    if (!widget) {
        THROW ("couldn't find widget '"
               + a_widget_name
               + "' in glade file: " + a_glade->get_filename ().c_str ());
    }
    T *result = dynamic_cast<T*> (widget) ;
    if (!result) {
        //TODO: we may leak widget here if it is a toplevel widget
        //like Gtk::Window. In this case, we should make sure to delete it
        //before bailing out.
        THROW ("widget " + a_widget_name + " is not of the expected type") ;
    }
    return result ;
}

template <class T>
T*
get_widget_from_glade (const UString &a_glade_file_name,
                       const UString &a_widget_name,
                       const Glib::RefPtr<Gnome::Glade::Xml> &a_glade)
{
    UString path_to_glade_file =
                common::env::build_path_to_glade_file (a_glade_file_name) ;
    a_glade = Gnome::Glade::Xml::create (path_to_glade_file) ;
    if (!a_glade) {
        THROW ("Could not create glade from file " + path_to_glade_file) ;
    }
    return get_widget_from_glade<T> (a_glade, a_widget_name) ;
}

#ifndef NEMIVER_TRY
#define NEMIVER_TRY try {
#endif

#ifndef NEMIVER_CATCH_NOX
#define NEMIVER_CATCH_NOX \
} catch (Glib::Exception &e) { \
    LOG_ERROR (e.what ()) ; \
} catch (std::exception &e) { \
    LOG_ERROR (e.what ()) ; \
} catch (...) { \
    LOG_ERROR ("An unknown error occured") ; \
}
#endif

#ifndef NEMIVER_CATCH_AND_RETURN_NOX
#define NEMIVER_CATCH_AND_RETURN_NOX(a_value) \
} catch (Glib::Exception &e) { \
    LOG_ERROR (e.what ()) ; \
    return a_value ; \
} catch (std::exception &e) { \
    LOG_ERROR (e.what ()) ; \
    return a_value ; \
} catch (...) { \
    LOG_ERROR ("An unknown error occured") ; \
    return a_value ; \
}
#endif

#ifndef NEMIVER_BEGIN_NAMESPACE
#define NEMIVER_BEGIN_NAMESPACE(name) namespace name {
#endif


#ifndef NEMIVER_END_NAMESPACE
#define NEMIVER_END_NAMESPACE }
#endif


}//end namespace env
}//end namespace common
}//end namespace nemiver

#endif //__NEMIVER_ENV_H__

