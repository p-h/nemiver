/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4; -*- */

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
 *If not, see <http://www.gnu.org/licenses/>.
 *
 *See COPYRIGHT file copyright information.
 */
#ifndef __NMV_DELETE_STATEMENT_H__
#define __NMV_DELETE_STATEMENT_H__

#include "nmv-sql-statement.h"

namespace nemiver {
namespace common {

struct DeleteStatementPriv;
class NEMIVER_API DeleteStatement : public SQLStatement
{
    friend struct DeleteStatementPriv;

    DeleteStatementPriv *m_priv;
    //forbid copy/assignation
    DeleteStatement (const DeleteStatement &);
    DeleteStatement& operator= (const DeleteStatement &);

public:

    DeleteStatement (const common::UString &a_table_name,
                     ColumnList &a_where_columns);

    ~DeleteStatement ();

    const common::UString& to_string () const;

    const ColumnList& get_where_columns () const;
};//end class DeleteStatement

}//end namespace common
}//end namespace nemiver

#endif //__NMV_DELETE_STATEMENT_H__

