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
#include "config.h"
#include <iostream>
#include "common/nmv-safe-ptr-utils.h"
#include "common/nmv-connection.h"
#include "common/nmv-connection-manager.h"
#include "common/nmv-tools.h"
#include "common/nmv-transaction.h"
#include "common/nmv-sql-statement.h"
#include "common/nmv-conf-manager.h"
#include "nmv-ui-utils.h"
#include "nmv-sess-mgr.h"

using namespace std;
using namespace nemiver::common;
using nemiver::common::Connection;
using nemiver::common::ConnectionSafePtr;
using nemiver::common::ConnectionManager;
using nemiver::common::Transaction;
using nemiver::common::SQLStatement;

static const char *REQUIRED_DB_SCHEMA_VERSION = "1.5";
static const char *DB_FILE_NAME = "nemivercommon.db";

NEMIVER_BEGIN_NAMESPACE (nemiver)

class SessMgr : public ISessMgr {
    //non copyable
    SessMgr (const SessMgr&);
    SessMgr& operator= (const SessMgr&);
    struct Priv;
    SafePtr<Priv> m_priv;

protected:
    SessMgr ();

public:
    SessMgr (const UString &root_dir);
    virtual ~SessMgr ();
    Transaction& default_transaction ();
    list<Session>& sessions ();
    const list<Session>& sessions () const;
    void store_session (Session &a_session,
                        Transaction &a_trans);
    void store_sessions (Transaction &a_trans);
    void load_session (Session &a_session,
                       Transaction &a_trans);
    void load_sessions (Transaction &a_trans);
    void load_sessions ();
    void delete_session (gint64 a_id,
                         Transaction &a_trans);
    void delete_session (gint64 a_id);
    void delete_sessions (Transaction &a_trans);
    void delete_sessions ();
    void clear_session (gint64 a_id, Transaction &a_trans);
    void clear_session (gint64 a_id);
};//end class SessMgr

struct SessMgr::Priv {
    UString root_dir;
    list<Session> sessions;
    ConnectionSafePtr conn;
    TransactionSafePtr default_transaction;

    Priv () {}
    Priv (const UString &a_root_dir) :
        root_dir (a_root_dir)
    {}

    ConnectionSafePtr connection ()
    {
        if (!conn) {
            conn = ConnectionManager::create_db_connection ();
        }
        THROW_IF_FAIL (conn);
        return conn;
    }

    UString path_to_create_tables_script ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        string path = Glib::build_filename
                                    (Glib::locale_from_utf8 (root_dir),
                                     "sqlscripts/create-tables.sql");
        return Glib::locale_to_utf8 (path);
    }

    UString path_to_drop_tables_script ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        string path = Glib::build_filename
                                    (Glib::locale_from_utf8 (root_dir),
                                     "sqlscripts/drop-tables.sql");
        return Glib::locale_to_utf8 (path);
    }

    const string& get_db_file_path () const
    {
        static string db_file_path;
        if (db_file_path.empty ()) {
            vector<string> path_elems;
            path_elems.push_back (ConfManager::get_user_config_dir_path ());
            path_elems.push_back (DB_FILE_NAME);
            db_file_path = Glib::build_filename (path_elems);
        }
        LOG_DD ("db_file_path: " << db_file_path);
        return db_file_path;
    }

    bool db_file_path_exists () const
    {
        if (Glib::file_test (get_db_file_path (), Glib::FILE_TEST_EXISTS)) {
            LOG_DD ("could not find file: " << get_db_file_path ());
            return true;
        }
        return false;
    }

    bool create_db ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        UString path_to_script = path_to_create_tables_script ();
        Transaction transaction (*connection ());
        return tools::execute_sql_command_file
                                        (path_to_script,
                                         transaction,
                                         cerr);
    }

    bool drop_db ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        UString path_to_script = path_to_drop_tables_script ();
        Transaction transaction (*connection ());
        return tools::execute_sql_command_file
                                        (path_to_script,
                                         transaction,
                                         cerr);
    }

    bool check_db_version ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        NEMIVER_TRY

        SQLStatement query ("select version from schemainfo");

        RETURN_VAL_IF_FAIL (connection ()->execute_statement (query), false);
        RETURN_VAL_IF_FAIL (connection ()->read_next_row (), false);
        UString version;
        RETURN_VAL_IF_FAIL (connection ()->get_column_content (0, version),
                            false);
        LOG_DD ("version: " << version);
        if (version != REQUIRED_DB_SCHEMA_VERSION) {
            return false;
        }
        NEMIVER_CATCH_AND_RETURN (false)
        return true;
    }

    void init_db ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY

        // If there is not db, create a new one with 
        // the schema we expect.
        if (!db_file_path_exists ()) {
            THROW_IF_FAIL (create_db ());
        } else if (!check_db_version ()) {
            // If the db version is not what we expect, create
            // a new db with the schema we expect.
            drop_db ();
            THROW_IF_FAIL (create_db ());
        }
        NEMIVER_CATCH
    }

    void init ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY
        init_db ();
        NEMIVER_CATCH
    }
};//end struct SessMgr::Priv

SessMgr::SessMgr ()
{
    m_priv.reset (new SessMgr::Priv);
    m_priv->init ();
}

SessMgr::SessMgr (const UString &a_root_dir)
{
    m_priv.reset (new SessMgr::Priv (a_root_dir));
    m_priv->init ();
}

SessMgr::~SessMgr ()
{
    LOG_D ("delete", "destructor-domain");
}

Transaction&
SessMgr::default_transaction ()
{
    THROW_IF_FAIL (m_priv);

    if (!m_priv->default_transaction) {
        m_priv->default_transaction =
            TransactionSafePtr (new Transaction (*m_priv->connection ()));
        THROW_IF_FAIL (m_priv->default_transaction);
    }
    return *m_priv->default_transaction;
}

list<SessMgr::Session>&
SessMgr::sessions ()
{
    return m_priv->sessions;
}

const list<SessMgr::Session>&
SessMgr::sessions () const
{
    return m_priv->sessions;
}

void
SessMgr::store_session (Session &a_session,
                        Transaction &a_trans)
{
    THROW_IF_FAIL (m_priv);

    // The next line starts a transaction.
    // If we get off from this function without reaching
    // the trans.end() call, every db request we made gets rolled back.
    TransactionAutoHelper trans (a_trans);

    UString query;
    if (!a_session.session_id ()) {
        // insert the session id in the sessions table, and get the session id
        // we just inerted
        query = "insert into sessions values(NULL)";
        THROW_IF_FAIL2
            (trans.get ().get_connection ().execute_statement (query),
             "failed to execute query: '" + query + "'");
        query = "select max(id) from sessions";
        THROW_IF_FAIL2
            (trans.get ().get_connection ().execute_statement (query),
             "failed to execute query: '" + query + "'");
        LOG_DD ("query: " << query);
        THROW_IF_FAIL (trans.get ().get_connection ().read_next_row ());
        gint64 session_id = 0;
        THROW_IF_FAIL
            (trans.get ().get_connection ().get_column_content
                                                         (0, session_id));
        THROW_IF_FAIL (session_id);
        a_session.session_id (session_id);
    }

    // store the properties
    query = "delete from attributes where sessionid = "
            + UString::from_int (a_session.session_id ());
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    map<UString, UString>::const_iterator prop_iter;
    for (prop_iter = a_session.properties ().begin ();
         prop_iter != a_session.properties ().end ();
         ++prop_iter) {
        query = "insert into attributes values(NULL, "
                + UString::from_int (a_session.session_id ()) + ", '"
                + SQLStatement::escape_string (prop_iter->first) + "', '"
                + SQLStatement::escape_string (prop_iter->second)
                + "')";
        LOG_DD ("query: " << query);
        THROW_IF_FAIL
                (trans.get ().get_connection ().execute_statement (query));
    }

    // store the environment variables
    query = "delete from env_variables where sessionid = "
            + UString::from_int (a_session.session_id ());
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    map<UString, UString>::const_iterator var_iter;
    for (var_iter = a_session.env_variables ().begin ();
         var_iter != a_session.env_variables ().end ();
         ++var_iter) {
        query = "insert into env_variables values(NULL, "
                + UString::from_int (a_session.session_id ()) + ", '"
                + SQLStatement::escape_string (var_iter->first) + "', '"
                + SQLStatement::escape_string (var_iter->second)
                + "')";
        LOG_DD ("query: " << query);
        THROW_IF_FAIL
                (trans.get ().get_connection ().execute_statement (query));
    }

    // store the breakpoints
    query = "delete from breakpoints where sessionid = "
            + UString::from_int (a_session.session_id ());
    THROW_IF_FAIL
            (trans.get ().get_connection ().execute_statement (query));

    list<SessMgr::Breakpoint>::const_iterator break_iter;
    for (break_iter = a_session.breakpoints ().begin ();
         break_iter != a_session.breakpoints ().end ();
         ++break_iter) {
        UString condition = break_iter->condition ();
        condition.chomp ();
        query = "insert into breakpoints values(NULL, "
	  + UString::from_int (a_session.session_id ()) + ", '"
	  + SQLStatement::escape_string (break_iter->file_name ()) + "', '"
	  + SQLStatement::escape_string
	  (break_iter->file_full_name ()) + "', "
	  + UString::from_int (break_iter->line_number ()) + ", "
	  + UString::from_int (break_iter->enabled ()) + ", "
	  + "'" + SQLStatement::escape_string (condition) + "'" + ", "
	  + UString::from_int (break_iter->ignore_count ()) + ", "
	  + UString::from_int (break_iter->is_countpoint ())
	  + ")";
        LOG_DD ("query: " << query);
        THROW_IF_FAIL
                (trans.get ().get_connection ().execute_statement (query));
    }

    // store the watchpoints
    query = "delete from watchpoints where sessionid = "
            + UString::from_int (a_session.session_id ());
    THROW_IF_FAIL
            (trans.get ().get_connection ().execute_statement (query));

    list<SessMgr::WatchPoint>::const_iterator watch_iter;
    for (watch_iter = a_session.watchpoints ().begin ();
         watch_iter != a_session.watchpoints ().end ();
         ++watch_iter) {
        UString expression = watch_iter->expression ();
        expression.chomp ();
        query = "insert into watchpoints values(NULL, "
                + UString::from_int (a_session.session_id ()) + ", '"
                + SQLStatement::escape_string (expression) + "', "
                + UString::from_int (watch_iter->is_read ()) + ", "
                + UString::from_int (watch_iter->is_write ())
                + ")";
        LOG_DD ("query: " << query);
        THROW_IF_FAIL
                (trans.get ().get_connection ().execute_statement (query));
    }

    // store the opened files
    query = "delete from openedfiles where sessionid = "
            + UString::from_int (a_session.session_id ());
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    list<UString>::const_iterator ofile_iter;
    for (ofile_iter = a_session.opened_files ().begin ();
         ofile_iter != a_session.opened_files ().end ();
         ++ofile_iter) {
        query = "insert into openedfiles values(NULL, "
                + UString::from_int (a_session.session_id ()) + ", '"
                + SQLStatement::escape_string (*ofile_iter)
                + "')";
        LOG_DD ("query: " << query);
        THROW_IF_FAIL
                (trans.get ().get_connection ().execute_statement (query));
    }

    // store the search paths
    query = "delete from searchpaths where sessionid = "
            + UString::from_int (a_session.session_id ());
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    list<UString>::const_iterator path_iter;
    for (path_iter = a_session.search_paths ().begin ();
         path_iter != a_session.search_paths ().end ();
         ++path_iter) {
        query = "insert into searchpaths values(NULL, "
                + UString::from_int (a_session.session_id ()) + ", '"
                + SQLStatement::escape_string (*path_iter)
                + "')";
        LOG_DD ("query: " << query);
        THROW_IF_FAIL
                (trans.get ().get_connection ().execute_statement (query));
    }
    trans.end ();
}

void
SessMgr::store_sessions (Transaction &a_trans)
{
    list<Session>::iterator session_iter;
    for (session_iter = sessions ().begin ();
         session_iter != sessions ().end ();
         ++session_iter) {
        store_session (*session_iter, a_trans);
    }
}

void
SessMgr::load_session (Session &a_session,
                       Transaction &a_trans)
{
    if (!a_session.session_id ()) {
        THROW ("Session has null ID. Can't load if from database");
    }

    THROW_IF_FAIL (m_priv);

    Session session;
    session.session_id (a_session.session_id ());

    // The next line starts a transaction.
    // If we get off from this function without reaching
    // the trans.end() call, every db request we made gets rolled back.
    TransactionAutoHelper trans (a_trans);

    // load the attributes
    UString query="select attributes.name, attributes.value "
                  "from attributes where attributes.sessionid = "
                  + UString::from_int (a_session.session_id ());
    LOG_DD ("query: " << query);
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));
    while (trans.get ().get_connection ().read_next_row ()) {
        UString name, value;
        THROW_IF_FAIL
            (trans.get ().get_connection ().get_column_content (0, name));
        THROW_IF_FAIL
            (trans.get ().get_connection ().get_column_content (1, value));
        session.properties ()[name] = value;
    }

    // load the environment variables
    query = "select env_variables.name, env_variables.value "
            "from env_variables where env_variables.sessionid = "
                  + UString::from_int (a_session.session_id ());
    LOG_DD ("query: " << query);
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));
    while (trans.get ().get_connection ().read_next_row ()) {
        UString name, value;
        THROW_IF_FAIL
            (trans.get ().get_connection ().get_column_content (0, name));
        THROW_IF_FAIL
            (trans.get ().get_connection ().get_column_content (1, value));
        session.env_variables ()[name] = value;
    }

    // load the breakpoints
    query = "select breakpoints.filename, breakpoints.filefullname, "
      "breakpoints.linenumber, breakpoints.enabled, "
      "breakpoints.condition, breakpoints.ignorecount,"
      "breakpoints.iscountpoint from "
      "breakpoints where breakpoints.sessionid = "
      + UString::from_int (session.session_id ());

    LOG_DD ("query: " << query);
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));
    while (trans.get ().get_connection ().read_next_row ()) {
        UString filename, filefullname, linenumber,
	  enabled, condition, ignorecount, is_countpoint;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                            (0, filename));
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                        (1, filefullname));
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                            (2, linenumber));
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                            (3, enabled));
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                            (4, condition));
        condition.chomp ();
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                            (5, ignorecount));
	THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                            (6, is_countpoint));
        LOG_DD ("filename, filefullname, linenumber, enabled, "
                "condition, ignorecount:\n"
                << filename << "," << filefullname << ","
                << linenumber << "," << enabled << ","
                << condition << "," << ignorecount
		<< is_countpoint);
        session.breakpoints ().push_back (SessMgr::Breakpoint (filename,
                                                               filefullname,
                                                               linenumber,
                                                               enabled,
                                                               condition,
                                                               ignorecount,
							       is_countpoint));
    }

    // load the watchpoints
    query = "select watchpoints.expression, watchpoints.iswrite, "
            "watchpoints.isread from watchpoints where watchpoints.sessionid = "
            + UString::from_int (session.session_id ());
    LOG_DD ("query: " << query);
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));
    while (trans.get ().get_connection ().read_next_row ()) {
        UString expression;
        gint64 is_write = false, is_read = false;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                            (0, expression));
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                        (1, is_write));
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                            (2, is_read));
        session.watchpoints ().push_back (SessMgr::WatchPoint (expression,
                                                               is_write,
                                                               is_read));
    }

    // load the search paths
    query = "select searchpaths.path from "
            "searchpaths where searchpaths.sessionid = "
            + UString::from_int (session.session_id ());
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));
    while (trans.get ().get_connection ().read_next_row ()) {
        UString path;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                                (0, path));
        session.search_paths ().push_back (path);
    }

    // load the opened files
    query = "select openedfiles.filename from openedfiles where "
            "openedfiles.sessionid = "
            + UString::from_int (session.session_id ());
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    while (trans.get ().get_connection ().read_next_row ()) {
        UString filename;
        THROW_IF_FAIL (trans.get ().get_connection ().get_column_content
                                                                (0, filename));
        session.opened_files ().push_back (filename);
    }

    trans.end ();
    a_session = session;
}

void
SessMgr::load_sessions (Transaction &a_trans)
{
    THROW_IF_FAIL (m_priv);
    UString query = "select sessions.id from sessions";

    TransactionAutoHelper trans (a_trans);

    list<Session> sessions;
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));
    while (trans.get ().get_connection ().read_next_row ()) {
        gint64 session_id=0;
        trans.get ().get_connection ().get_column_content (0, session_id);
        THROW_IF_FAIL (session_id);
        sessions.push_back (Session (session_id));
    }
    list<Session>::iterator session_iter;
    for (session_iter = sessions.begin ();
         session_iter != sessions.end ();
         ++session_iter) {
        load_session (*session_iter, default_transaction ());
    }
    m_priv->sessions = sessions;
    trans.end ();
}

void
SessMgr::load_sessions ()
{
    load_sessions (default_transaction ());
}

void
SessMgr::delete_session (gint64 a_id,
                         Transaction &a_trans)
{
    THROW_IF_FAIL (m_priv);

    TransactionAutoHelper trans (a_trans);

    clear_session (a_id, a_trans);
    UString query = "delete from sessions where "
                    "id = " + UString::from_int (a_id);
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    trans.end ();
}

void
SessMgr::delete_session (gint64 a_id)
{
    delete_session (a_id, default_transaction ());
}

void
SessMgr::delete_sessions (Transaction &a_trans)
{
    list<Session>::const_iterator iter;
    for (iter = sessions ().begin ();
         iter != sessions ().end ();
         ++iter) {
        delete_session (iter->session_id (),
                        a_trans);
    }
}

void
SessMgr::delete_sessions ()
{
    delete_sessions (default_transaction ());
}

void
SessMgr::clear_session (gint64 a_id, Transaction &a_trans)
{
    THROW_IF_FAIL (m_priv);
    TransactionAutoHelper trans (a_trans);

    UString query = "delete from attributes where "
                    "sessionid = " + UString::from_int (a_id);
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    query = "delete from breakpoints where "
            "sessionid = " + UString::from_int (a_id);
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    query = "delete from openedfiles where "
            "sessionid = " + UString::from_int (a_id);
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    query = "delete from searchpaths where "
            "sessionid = " + UString::from_int (a_id);
    THROW_IF_FAIL (trans.get ().get_connection ().execute_statement (query));

    trans.end ();
}

void
SessMgr::clear_session (gint64 a_id)
{
    clear_session (a_id, default_transaction ());
}

ISessMgrSafePtr
ISessMgr::create (const UString &a_root_path)
{
    return ISessMgrSafePtr (new SessMgr (a_root_path));
}

NEMIVER_END_NAMESPACE (nemiver)


