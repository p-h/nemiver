/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4 -*- */

/*
 *This file is part of the Nemiver Project.
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
#include <sqlite3.h>
#include "nmv-ustring.h"
#include "nmv-exception.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-sqlite-cnx-drv.h"
#include "nmv-sqlite-cnx-mgr-drv.h"
#include "nmv-env.h"

using namespace nemiver::common ;

extern "C" {

bool
NEMIVER_API
nemiver_common_create_dynamic_module_instance (void **a_new_instance)
{
    RETURN_VAL_IF_FAIL (a_new_instance, false) ;

    try {
        nemiver::common::IConnectionManagerDriverSafePtr manager
            (nemiver::common::sqlite::SqliteCnxMgrDrv::get_connection_manager_driver ());
        *a_new_instance = manager.ref_and_get () ;

    } catch (std::exception &e) {
        TRACE_EXCEPTION (e) ;
        return false ;
    } catch (Glib::Exception &e) {
        TRACE_EXCEPTION (e) ;
        return false ;
    } catch (...) {
        LOG ("Got an unknown exception") ;
        return false ;
    }
    return true ;
}
}//end extern C

namespace nemiver {
namespace common {
namespace sqlite {

struct SqliteCnxMgrDrvPriv {
};//end SqliteCnxMgrDrvPriv

SqliteCnxMgrDrv::SqliteCnxMgrDrv ()
{
    m_priv = new SqliteCnxMgrDrvPriv () ;
}

SqliteCnxMgrDrv::~SqliteCnxMgrDrv ()
{
    if (!m_priv) {
        return ;
    }

    delete m_priv ;
    m_priv = NULL ;
}

void
SqliteCnxMgrDrv::get_info (Info &a_info) const
{
    a_info.module_name = "org.nemiver.db.sqlitedriver.default" ;
    a_info.module_description = "The nemiver database driver for sqlite."
                                " Implements the IConnectionManagerDriver iface" ;
    a_info.module_version = "0.0.1" ;
}

IConnectionDriverSafePtr
SqliteCnxMgrDrv::connect_to_db (const DBDesc &a_db_desc,
                                const UString &a_user,
                                const UString &a_pass)
{

    if (a_user == "") {}
    if (a_pass == "") {}
    sqlite3 *sqlite (NULL);

    //HACK. As we are using sqlite, make sure to use a db file
    //that is in $HOME/.nemiver/db/sqlite
    UString db_name (a_db_desc.name ()) ;
    if (!Glib::path_is_absolute (db_name)) {
        if (!Glib::file_test (env::get_user_db_dir (),
                              Glib::FILE_TEST_IS_DIR)) {
            env::create_user_db_dir () ;
        }
        db_name = Glib::build_filename (env::get_user_db_dir (),
                                        db_name).c_str () ;
    }

    int result = sqlite3_open (db_name.c_str (), &sqlite) ;
    if (result != SQLITE_OK) {
        THROW ("could not connect to sqlite database: "
               + UString (sqlite3_errmsg(sqlite))) ;
        sqlite3_close (sqlite);
        exit(1);
    }
    common::IConnectionDriverSafePtr connection_driver
                                            (new SqliteCnxDrv (sqlite)) ;
    return connection_driver ;
}

IConnectionManagerDriverSafePtr
SqliteCnxMgrDrv::get_connection_manager_driver ()
{
    static IConnectionManagerDriverSafePtr s_connection_manager_driver (NULL);
    if (!s_connection_manager_driver) {
        s_connection_manager_driver = new SqliteCnxMgrDrv ;
    }
    return s_connection_manager_driver ;
}

void
SqliteCnxMgrDrv::do_init ()
{
}

}//end sqlite
}//end namespace common
}//end namespace  nemiver

