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

static const char *REQUIRED_DB_SCHEMA_VERSION = "1.0" ;

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
        if (!conn) {
            conn= ConnectionManager::create_db_connection () ;
        }
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
        UString version ;
        THROW_IF_FAIL (connection ()->get_column_content (0, version)) ;
        LOG ("version: " << version) ;
        if (version != REQUIRED_DB_SCHEMA_VERSION) {
            return false ;
        }
        return true ;
    }

    void init_db ()
    {
        //if the db version is not what we expect, create
        //a new db with the schema we expect.
        if (!check_db_version ()) {
            THROW_IF_FAIL (create_db ()) ;
        }
    }

    void init ()
    {
        init_db () ;
    }
};//end struct SessMgr::Priv

SessMgr::SessMgr ()
{
    m_priv = new SessMgr::Priv ;
    m_priv->init () ;
}

SessMgr::SessMgr (const UString &a_root_dir)
{
    m_priv = new SessMgr::Priv (a_root_dir) ;
    m_priv->init () ;
}

list<SessMgr::Session>&
SessMgr::sessions ()
{
    return m_priv->sessions ;
}

static const char * SESSION_NAME = "sessionname" ;

void
SessMgr::store_session (const Session &a_session)
{
    THROW_IF_FAIL (m_priv) ;

    Transaction transaction (*m_priv->connection ());

    //The next line starts a transaction.
    //If we get off from this function without reaching
    //the trans.end() call, every db request we made gets rolled back.
    TransactionAutoHelper trans (transaction) ;

    //insert the session id in the sessions table, and get the session id
    //we just inerted
    UString query ("insert into sessions values(NULL)");
    THROW_IF_FAIL2 (trans.get ().get_connection ().execute_statement (query),
                    "failed to execute query: '" + query + "'") ;
    query = "select max(id) from sessions" ;
    THROW_IF_FAIL2 (trans.get ().get_connection ().execute_statement (query),
                    "failed to execute query: '" + query + "'") ;
    THROW_IF_FAIL (trans.get ().get_connection ().read_next_row ()) ;
    gint64 session_id=0 ;
    THROW_IF_FAIL (trans.get ().get_connection ().get_column_content (0, session_id)) ;
    THROW_IF_FAIL (session_id) ;

    //store the session name
    query = "insert into attributes values(NULL, "
            + UString::from_int (session_id) + ", "
            + SESSION_NAME + ", "
            + a_session.name()
            + ")"
            ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;

    //store the breakpoints
    list<IDebugger::BreakPoint>::const_iterator iter ;
    for (iter = a_session.breakpoints ().begin ();
         iter != a_session.breakpoints ().end ();
         ++iter) {
        query = "insert into breakpoints values(NULL, "
                + UString::from_int (session_id) + ", "
                + iter->full_file_name () + ", "
                + UString::from_int (iter->line ())
                + ")"
                ;
        THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;
    }

    //TODO: finish this ! (store the opened files)
    trans.end () ;
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


