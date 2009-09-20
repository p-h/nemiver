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
#ifdef __NMV_PREF_MGR_H__
#define __NMV_PREF_MGR_H__

#include <list>
#include <boost/variant.hpp>
#include "common/nmv-dynamic-module.h"
#include "common/nmv-ustring.h"

using namespace std;
using nemiver::common::UString;
using boost::variant;

namespace nemiver {

/// \brief the interface of the preference manager.
/// preferences must be set/get through this object.
class NEMIVER_API IPrefMgr : public DynamicModule {

    //non copyable
    IPrefMgr (const IPrefMgr&);
    IPrefMgr& operator= (const IPrefMgr&);

protected:
    //must be created by the dynamic module factory
    IPrefMgr () {}

public:
    class Pref {
        UString m_name;
        boost::variant<gint32, UString> m_value;

    public:
        enum Type {
            INT=0,
            STRING=1
        };

        Pref (gint32 a_in) :
            m_value (a_in)
        {}

        Pref (const UString &a_in) :
            m_value (a_in)
        {}

        void value (gint32 a_in) {m_value = a_in;}
        void value (const UString &a_in) {m_value = a_in;}
        const variant& value () {return m_value;}

        void name (const UString &a_in) {a_in = a_name;}
        const UString& name () {return m_name;}
    };//end class pref

    virtual ~IPrefMgr () {}

};//end class IPrefMgr

}//end namespace nemiver

#endif //__NMV_PREF_MGR_H__

