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
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#include "nmv-safe-ptr-utils.h"
#include "nmv-sess-mgr.h"
#include "nmv-connection.h"
#include "nmv-connection-manager.h"
#include "nmv-tools.h"
#include "nmv-transaction.h"
#include "nmv-sql-statement.h"

using namespace std ;
using namespace nemiver::common ;
using nemiver::common::Connection ;
using nemiver::common::ConnectionSafePtr ;
using nemiver::common::ConnectionManager ;
using nemiver::common::Transaction ;
using nemiver::common::SQLStatement ;

namespace nemiver {

struct SessMgr::Priv {
    UString root_dir ;
    list<Session> sessions ;
    ConnectionSafePtr conn ;

    Priv () {}
    Priv (const UString &a_root_dir) :
        root_dir (a_root_dir)
    {}

    ConnectionSafePtr connection ()
    {
        THROW_IF_FAIL (conn) ;
        return conn;
    }

    UString path_to_create_table_script ()
    {
        string path = Glib::build_filename
                                    (Glib::locale_from_utf8 (root_dir),
                                     "sqlscripts/sqlite/create-tables.sql") ;
        return Glib::locale_to_utf8 (path) ;
    }

    bool create_db ()
    {
        if (!conn) {
            conn= ConnectionManager::create_db_connection () ;
            THROW_IF_FAIL (conn) ;
        }

        UString path_to_script = path_to_create_table_script () ;
        Transaction transaction (*conn) ;
        return tools::execute_sql_command_file
                                        (path_to_create_table_script (),
                                         transaction,
                                         cerr) ;
    }

    bool check_db_version ()
    {
        SQLStatement insert_statement ("select version from schemainfo") ;

        THROW_IF_FAIL (connection ()->execute_statement (insert_statement)) ;
        THROW_IF_FAIL (connection ()->read_next_row ()) ;

        return false ;
    }

    void init_db ()
    {
    }
};//end struct SessMgr::Priv

SessMgr::SessMgr ()
{
    m_priv = new SessMgr::Priv ;
}

SessMgr::SessMgr (const UString &a_root_dir)
{
    m_priv = new SessMgr::Priv (a_root_dir) ;
}

list<SessMgr::Session>&
SessMgr::sessions ()
{
    return m_priv->sessions ;
}

void
SessMgr::store_sessions ()
{
}

void
SessMgr::load_sessions ()
{
}

}//end namespace nemiver


