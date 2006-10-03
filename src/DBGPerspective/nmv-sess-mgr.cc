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

class SessMgr : public ISessMgr {
    //non copyable
    SessMgr (const SessMgr&) ;
    SessMgr& operator= (const SessMgr&) ;
    struct Priv ;
    SafePtr<Priv> m_priv ;

protected:
    SessMgr () ;

public:
    SessMgr (const UString &root_dir) ;
    virtual ~SessMgr () ;
    Transaction& default_transaction () ;
    list<Session>& sessions () ;
    const list<Session>& sessions () const ;
    void store_session (Session &a_session,
                        Transaction &a_trans) ;
    void store_sessions (Transaction &a_trans) ;
    void load_session (Session &a_session,
                       Transaction &a_trans) ;
    void load_sessions (Transaction &a_trans);
    void load_sessions ();
    void delete_session (gint64 a_id,
                         Transaction &a_trans) ;
    void delete_session (gint64 a_id) ;
    void delete_sessions (Transaction &a_trans) ;
    void delete_sessions () ;
    void clear_session (gint64 a_id, Transaction &a_trans) ;
    void clear_session (gint64 a_id) ;
};//end class SessMgr

struct SessMgr::Priv {
    UString root_dir ;
    list<Session> sessions ;
    ConnectionSafePtr conn ;
    TransactionSafePtr default_transaction ;

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
                                     "sqlscripts/create-tables.sql") ;
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
        SQLStatement query ("select version from schemainfo") ;

        RETURN_VAL_IF_FAIL (connection ()->execute_statement (query), false) ;
        RETURN_VAL_IF_FAIL (connection ()->read_next_row (), false) ;
        UString version ;
        RETURN_VAL_IF_FAIL (connection ()->get_column_content (0, version),
                            false) ;
        LOG_D ("version: " << version, NMV_DEFAULT_DOMAIN) ;
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

SessMgr::~SessMgr ()
{
}

Transaction&
SessMgr::default_transaction ()
{
    THROW_IF_FAIL (m_priv) ;

    if (!m_priv->default_transaction) {
        m_priv->default_transaction = new Transaction (*m_priv->connection ()) ;
        THROW_IF_FAIL (m_priv->default_transaction) ;
    }
    return *m_priv->default_transaction ;
}

list<SessMgr::Session>&
SessMgr::sessions ()
{
    return m_priv->sessions ;
}

const list<SessMgr::Session>&
SessMgr::sessions () const
{
    return m_priv->sessions ;
}

void
SessMgr::store_session (Session &a_session,
                        Transaction &a_trans)
{
    THROW_IF_FAIL (m_priv) ;

    //The next line starts a transaction.
    //If we get off from this function without reaching
    //the trans.end() call, every db request we made gets rolled back.
    TransactionAutoHelper trans (a_trans) ;

    UString query ;
    if (!a_session.session_id ()) {
        //insert the session id in the sessions table, and get the session id
        //we just inerted
        query = "insert into sessions values(NULL)";
        THROW_IF_FAIL2 (trans.get ().get_connection ().execute_statement (query),
                        "failed to execute query: '" + query + "'") ;
        query = "select max(id) from sessions" ;
        THROW_IF_FAIL2 (trans.get ().get_connection ().execute_statement (query),
                        "failed to execute query: '" + query + "'") ;
        THROW_IF_FAIL (trans.get ().get_connection ().read_next_row ()) ;
        gint64 session_id=0 ;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content (0, session_id)) ;
        THROW_IF_FAIL (session_id) ;
        a_session.session_id (session_id) ;
    }

    //store the properties
    query = "delete from attributes where sessionid = "
            + UString::from_int (a_session.session_id ()) ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    map<UString, UString>::const_iterator prop_iter ;
    for (prop_iter = a_session.properties ().begin ();
         prop_iter != a_session.properties ().end ();
         ++prop_iter) {
        query = "insert into attributes values(NULL, "
                + UString::from_int (a_session.session_id ()) + ", '"
                + prop_iter->first + "', '"
                + prop_iter->second
                + "')"
                ;
        THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));
    }

    //store the environment variables
    query = "delete from env_variables where sessionid = "
            + UString::from_int (a_session.session_id ()) ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    map<UString, UString>::const_iterator var_iter ;
    for (var_iter = a_session.env_variables ().begin ();
         var_iter != a_session.env_variables ().end ();
         ++var_iter) {
        query = "insert into env_variables values(NULL, "
                + UString::from_int (a_session.session_id ()) + ", '"
                + var_iter->first + "', '"
                + var_iter->second
                + "')"
                ;
        THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));
    }

    //store the breakpoints
    query = "delete from breakpoints where sessionid = "
            + UString::from_int (a_session.session_id ()) ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    list<SessMgr::BreakPoint>::const_iterator break_iter ;
    for (break_iter = a_session.breakpoints ().begin ();
         break_iter != a_session.breakpoints ().end ();
         ++break_iter) {
        query = "insert into breakpoints values(NULL, "
                + UString::from_int (a_session.session_id ()) + ", '"
                + break_iter->file_name () + "', '"
                + break_iter->file_full_name () + "', "
                + UString::from_int (break_iter->line_number ())
                + ")"
                ;
        THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;
    }

    //store the opened files
    query = "delete from openedfiles where sessionid = "
            + UString::from_int (a_session.session_id ()) ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    list<UString>::const_iterator ofile_iter ;
    for (ofile_iter = a_session.opened_files ().begin ();
         ofile_iter != a_session.opened_files ().end ();
         ++ofile_iter) {
        query = "insert into openedfiles values(NULL, "
                + UString::from_int (a_session.session_id ()) + ", '"
                + *ofile_iter
                + "')" ;
        THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;
    }
    trans.end () ;
}

void
SessMgr::store_sessions (Transaction &a_trans)
{
    list<Session>::iterator session_iter ;
    for (session_iter = sessions ().begin ();
         session_iter != sessions ().end ();
         ++session_iter) {
        store_session (*session_iter, a_trans) ;
    }
}

void
SessMgr::load_session (Session &a_session,
                       Transaction &a_trans)
{
    if (!a_session.session_id ()) {
        THROW ("Session has null ID. Can't load if from database") ;
    }

    THROW_IF_FAIL (m_priv) ;

    Session session ;
    session.session_id (a_session.session_id ()) ;

    //The next line starts a transaction.
    //If we get off from this function without reaching
    //the trans.end() call, every db request we made gets rolled back.
    TransactionAutoHelper trans (a_trans) ;

    //load the attributes
    UString query="select attributes.name, attributes.value "
                  "from attributes where attributes.sessionid = "
                  + UString::from_int (a_session.session_id ());
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;
    while (trans.get ().get_connection ().read_next_row ()) {
        UString name, value ;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content (0, name)) ;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content (1, value)) ;
        session.properties ()[name] = value ;
    }

    //load the environment variables
    query="select env_variables.name, env_variables.value "
                  "from env_variables where env_variables.sessionid = "
                  + UString::from_int (a_session.session_id ());
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;
    while (trans.get ().get_connection ().read_next_row ()) {
        UString name, value ;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content (0, name)) ;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content (1, value)) ;
        session.env_variables ()[name] = value ;
    }

    //load the breakpoints
    query = "select breakpoints.filename, breakpoints.filefullname, "
            "breakpoints.linenumber from "
            "breakpoints where breakpoints.sessionid = "
            + UString::from_int (session.session_id ())
            ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;
    while (trans.get ().get_connection ().read_next_row ()) {
        UString filename, filefullname, linenumber;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                                (0, filename));
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                            (1, filefullname));
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                                (2, linenumber));
        session.breakpoints ().push_back (SessMgr::BreakPoint (filename,
                                                               filefullname,
                                                               linenumber)) ;
    }

    //load the opened files
    query = "select openedfiles.filename from openedfiles where "
            "openedfiles.sessionid = "
            + UString::from_int (session.session_id ()) ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;

    while (trans.get ().get_connection ().read_next_row ()) {
        UString filename ;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                                (0, filename));
        session.opened_files ().push_back (filename) ;
    }

    trans.end () ;
    a_session = session ;
}

void
SessMgr::load_sessions (Transaction &a_trans)
{
    THROW_IF_FAIL (m_priv) ;
    UString query = "select sessions.id from sessions" ;

    TransactionAutoHelper trans (a_trans) ;

    list<Session> sessions ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;
    while (trans.get ().get_connection ().read_next_row ()) {
        gint64 session_id=0 ;
        trans.get ().get_connection ().get_column_content (0, session_id) ;
        THROW_IF_FAIL (session_id) ;
        sessions.push_back (Session (session_id)) ;
    }
    list<Session>::iterator session_iter ;
    for (session_iter = sessions.begin ();
         session_iter != sessions.end ();
         ++session_iter) {
        load_session (*session_iter, default_transaction ()) ;
    }
    m_priv->sessions = sessions ;
    trans.end () ;
}

void
SessMgr::load_sessions ()
{
    load_sessions (default_transaction ()) ;
}

void
SessMgr::delete_session (gint64 a_id,
                         Transaction &a_trans)
{
    THROW_IF_FAIL (m_priv) ;

    TransactionAutoHelper trans (a_trans) ;

    clear_session (a_id, a_trans) ;
    UString query = "delete from sessions where "
                    "id = " + UString::from_int (a_id) ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;

    trans.end () ;
}

void
SessMgr::delete_session (gint64 a_id)
{
    delete_session (a_id, default_transaction ()) ;
}

void
SessMgr::delete_sessions (Transaction &a_trans)
{
    list<Session>::const_iterator iter ;
    for (iter = sessions ().begin ();
         iter != sessions ().end ();
         ++iter) {
        delete_session (iter->session_id (),
                        a_trans) ;
    }
}

void
SessMgr::delete_sessions ()
{
    delete_sessions (default_transaction ()) ;
}

void
SessMgr::clear_session (gint64 a_id, Transaction &a_trans)
{
    THROW_IF_FAIL (m_priv) ;
    TransactionAutoHelper trans (a_trans) ;

    UString query = "delete from attributes where "
                    "sessionid = " + UString::from_int (a_id);
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;

    query = "delete from breakpoints where "
            "sessionid = " + UString::from_int (a_id) ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;

    query = "delete from openedfiles where "
            "sessionid = " + UString::from_int (a_id) ;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query)) ;

    trans.end () ;
}

void
SessMgr::clear_session (gint64 a_id)
{
    clear_session (a_id, default_transaction ()) ;
}

ISessMgrSafePtr
ISessMgr::create (const UString &a_root_path)
{
    return ISessMgrSafePtr (new SessMgr (a_root_path)) ;
}

}//end namespace nemiver


