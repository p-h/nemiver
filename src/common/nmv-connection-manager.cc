/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset: 4;  -*- */

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
#include <cstdlib>
#include <cstring>
#include <gmodule.h>
#include "nmv-exception.h"
#include "nmv-ustring.h"
#include "nmv-parsing-utils.h"
#include "nmv-log-stream-utils.h"
#include "nmv-i-connection-manager-driver.h"
#include "nmv-connection-manager.h"
#include "nmv-connection.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-conf-manager.h"
#include "nmv-dynamic-module.h"

using namespace nemiver::common;

namespace nemiver {

namespace common {

struct DBDriverDesc
{
    const common::UString driver_name;
    const common::UString module_name;
};

static bool parse_connection_string (const common::UString &a_str,
                                     common::DBDesc &a_desc);
static void load_db_driver_module (const common::DBDesc &a_desc);
static common::IConnectionManagerDriverSafePtr get_connection_manager_driver
                                                    (const common::DBDesc &);
static common::UString get_driver_module_name
    (const common::UString &a_driver_type);

static common::IConnectionManagerDriverSafePtr s_cnx_mgr_drv;

static common::UString s_db_type_loaded;

static DBDriverDesc s_supported_drivers [] = {
            {"mysql", "org.nemiver.db.mysqldriver"},
            {"sqlite", "org.nemiver.db.sqlitedriver"},
        };

static common::UString
get_driver_module_name (const common::UString &a_driver_name)
{
    if (a_driver_name == "")
        return "";
    for (unsigned int i=0;
         i < sizeof (s_supported_drivers)/sizeof (DBDriverDesc);
         ++i) {
        if (a_driver_name == s_supported_drivers[i].driver_name) {
            return s_supported_drivers[i].module_name;
        }
    }
    return "";
}

static common::DynamicModuleManager&
get_module_manager ()
{
    static DynamicModuleManager s_manager;
    return s_manager;
}

/// \brief parse a connection string.
/// this function returns a #DBDesc as the
/// result of the connection parsing.
/// the connection string has the form:
/// "vcommon" ':' DBTYPENAME ':' "//" [HOST[':'PORT]]'/'[SCHEMANAME]
/// DBTYPENAME ::= [a-zA-Z] [a-zA-Z0-9_.-]*
/// HOST ::= ([0-9.]+ | [a-zA-Z][a-zA-Z0-9_.-]+)
/// PORT ::= [0-9]+
/// SCHEMANAME ::= [a-zA-Z/][a-zA-Z0-9._/-]*
///
/// \param a_con_str the connection string to parse.
/// \param a_desc the database description returned as the
/// result of the parsing
/// \return true upon successful parsing, false otherwise.
static bool
parse_connection_string (const common::UString &a_str,
                         common::DBDesc &a_desc)
{
#define CHECK_INDEX(i) if ((i) >= a_str.size ()) {return false;}

    LOG_FUNCTION_SCOPE_NORMAL_DD;
    common::UString::size_type i = 5;
    common::UString dbtypename, host, port, dbschemaname;

    //must start with 'vdbc:'
    if (a_str.compare (0, 5, "vdbc:")) {
        return false;
    }

    //parse dbtypename
    bool parsed_dbtypename (false);
    if (!common::parsing_utils::is_alphabet_char (a_str[i])) {
        return false;
    }
    dbtypename += a_str[i++];
    for (; i < a_str.size (); ++i) {
        if (a_str[i] == ':') {
            parsed_dbtypename = true;
            ++i;
            CHECK_INDEX (i);
            break;
        }
        if (common::parsing_utils::is_alnum (a_str[i]) || a_str[i] == '.') {
            dbtypename += a_str[i];
        }
    }
    if (!parsed_dbtypename) {
        return false;
    }

    //now we must have "//"
    CHECK_INDEX (i);
    CHECK_INDEX (i + 1);
    if (!(a_str[i] == '/' && a_str[i + 1] == '/')) {
        return false;
    }
    i += 2;

    CHECK_INDEX (i);

    //now parse [HOST[:PORT]]
    if (a_str[i] == '/') {
        goto parse_schemaname;
    }
    if (common::parsing_utils::is_host_name_char (a_str[i])) {
        host += a_str[i];
        ++i;
        CHECK_INDEX (i);
        //parse HOST
        for (; i < a_str.size (); ++i) {
            if (a_str[i] == ':') {
                ++i;
                CHECK_INDEX (i);
                goto parse_port;
            };
            if (a_str[i] == '/') {
                ++i;
                CHECK_INDEX (i);
                goto parse_schemaname;
            }
            if (common::parsing_utils::is_host_name_char (a_str[i])) {
                host += a_str[i];
            } else {
                return false;
            }
        }

parse_port:
        bool parsed_port (false);
        for (; i < a_str.size (); ++i) {
            if (a_str[i] == '/') {
                parsed_port = true;
                ++i;
                CHECK_INDEX (i);
                goto parse_schemaname;
            }
            if (common::parsing_utils::is_digit (a_str[i])) {
                port += a_str[i];
            } else {
                return false;
            }
        }
        if (!parsed_port) {
            return false;
        }
    }

parse_schemaname:
    for (; i < a_str.size (); ++i) {
        if (common::parsing_utils::is_alnum (a_str[i])
            || a_str[i] == '_'
            || a_str[i] == '-'
            || a_str[i] == '.'
            || a_str[i] == '/') {
            dbschemaname += a_str[i];
        } else {
            return false;
        }
    }
    a_desc.set_type (dbtypename);
    a_desc.set_host (host);
    a_desc.set_port (atoi (port.c_str ()));
    a_desc.set_name (dbschemaname);
    return true;
}


static void
load_db_driver_module (const common::DBDesc &a_desc)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    common::UString driver_module_name = get_driver_module_name (a_desc.type ());
    if (driver_module_name == "") {
        THROW (UString ("database '") + a_desc.type () + "' is not supported");
    }

    s_cnx_mgr_drv =
        get_module_manager ().load_iface<IConnectionManagerDriver>
                                            (driver_module_name,
                                             "IConnectionManagerDriver");
    LOG_D ("cnx mgr refcount: " << (int) s_cnx_mgr_drv->get_refcount (),
           "refcount-domain");

    if (!s_cnx_mgr_drv) {
        THROW (UString ("db driver module ")
                + driver_module_name
                + "does not implement the interface "
                "nemiver::common::IConnectinManagerDriver");
    }
    s_db_type_loaded = a_desc.type ();
}

static common::IConnectionManagerDriverSafePtr
get_connection_manager_driver (const common::DBDesc &a_db_desc)
{
    if (!s_cnx_mgr_drv) {
        load_db_driver_module (a_db_desc);
        if (!s_cnx_mgr_drv) {
            THROW ("could not load the driver for database: "
                    + a_db_desc.type ());
        }
        if (s_db_type_loaded != a_db_desc.type ()) {
            THROW ("Loaded database driver mismatches with resqueted database. "
                   "Loaded: " + s_db_type_loaded +
                   "; requested: " + a_db_desc.name ());
        }
    }
    return s_cnx_mgr_drv;
}

void
ConnectionManager::create_db_connection (const common::UString &a_con_str,
                                         const common::UString &a_user,
                                         const common::UString &a_pass,
                                         Connection &a_connection)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    if (a_con_str == "") {
        THROW ("got connection string");
    }

    common::DBDesc db_desc;
    if (!parse_connection_string (a_con_str, db_desc)) {
        THROW ("failed to parse connection string: " + a_con_str);
    }

    common::IConnectionManagerDriverSafePtr driver =
        get_connection_manager_driver (db_desc);
    THROW_IF_FAIL (driver);

    common::IConnectionDriverSafePtr cnx_driver_iface =
        driver->connect_to_db (db_desc, a_user, a_pass);
    a_connection.set_connection_driver (cnx_driver_iface);
    a_connection.initialize ();
}

ConnectionSafePtr
ConnectionManager::create_db_connection ()
{
    common::UString connection_string, user, pass;

    common::ConfManager::get_config ().get_property ("database.connection",
            connection_string);
    common::ConfManager::get_config ().get_property ("database.username", user);
    common::ConfManager::get_config ().get_property ("database.password", pass);

    if (connection_string == "") {
        THROW ("Got connection string='';"
               " Conf manager is probably not initialized");
    }

    common::DBDesc db_desc;
    if (!parse_connection_string (connection_string, db_desc)) {
        THROW ("failed to parse connection string: " + connection_string);
    }
    common::IConnectionManagerDriverSafePtr driver =
        get_connection_manager_driver (db_desc);
    THROW_IF_FAIL (driver);

    common::IConnectionDriverSafePtr cnx_driver_iface =
        driver->connect_to_db (db_desc, user, pass);

    ConnectionSafePtr connection (new Connection ());
    connection->set_connection_driver (cnx_driver_iface);
    connection->initialize ();

    return connection;
}

const char*
ConnectionManager::get_db_type ()
{
    return s_db_type_loaded.c_str ();
}

}//end namespace common nemiver
}//end namespace nemiver

