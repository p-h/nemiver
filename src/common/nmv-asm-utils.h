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
#ifndef __NEMIVER_ASM_UTILS_H__
#define __NEMIVER_ASM_UTILS_H__

#include "nmv-asm-instr.h"

using nemiver::common::UString;

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (common)

template<class Stream>
Stream&
operator<< (Stream &a_out, const AsmInstr &a_instr)
{
    a_out << "<asm-instr>\n"
          << " <addr>" <<  a_instr.address () << "</addr>\n"
          << " <function-name>" << a_instr.function () << "</function-name>\n"
          << " <offset>" << a_instr.offset () << "</offset>\n"
          << " <instr>" << a_instr.instruction () << "</instr>\n"
          << "</asm-instr>\n";
    return a_out;
}

template<class Stream>
Stream&
operator<< (Stream &a_out, const MixedAsmInstr &a_instr)
{
    a_out << "<asm-mixed-instr>\n"
          << " <line>" << a_instr.line_number () << "</line>\n"
          << " <path>" << a_instr.file_path ()   << "</path>\n";

    list<AsmInstr>::const_iterator it;
    a_out << " <asm-instr-list>";
    for (it = a_instr.instrs ().begin ();
         it != a_instr.instrs ().end ();
         ++it) {
        a_out << "  <asm-instr>\n"
              << "   <addr>" <<  it->address () << "</addr>\n"
              << "   <function-name>" << it->function ()
                                      << "</function-name>\n"
              << "   <offset>" << it->offset ()      << "</offset>\n"
              << "   <instr>" <<  it->instruction () << "</instr>\n"
              << "  </asm-instr>\n";
    }
    a_out << " </asm-instr-list>"
          << "</asm-mixed-instr>\n";

    return a_out;
}

template<class Stream>
Stream&
operator<< (Stream &a_out, const Asm &a_asm)
{
    switch (a_asm.which ()) {
        case Asm::TYPE_PURE:
            a_out << a_asm.instr ();
            break;
        case Asm::TYPE_MIXED:
            a_out << a_asm.mixed_instr ();
            break;
        default:
	  THROW ("reached unreachable");
    }
    return a_out;
}

void log_asm_insns (const std::list<common::Asm> &a_asm);

typedef bool (* FindFileAndReadLine) (const UString &a_file_path,
                                      const UString &a_prog_path,
                                      const UString &a_cwd,
                                      list<UString> &a_sess_dirs,
                                      const list<UString> &a_glob_dirs,
                                      map<UString, bool> &a_ignore_paths,
                                      bool a_ignore_if_not_found,
                                      int a_line_number,
                                      std::string &a_line);
class ReadLine
{
 private:
    ReadLine ();
    ReadLine (const ReadLine &);

 protected:
    const UString &m_prog_path;
    const UString &m_cwd;
    list<UString> &m_session_dirs;
    const list<UString> &m_global_dirs;
    map<UString, bool> &m_ignore_paths;
    FindFileAndReadLine read_line;

 public:
    ReadLine (const UString &prog_path,
              const UString &cwd,
              list<UString> &session_dirs,
              const list<UString> &global_dirs,
              map<UString, bool> &ignore_paths,
              FindFileAndReadLine read_line_func) :
    m_prog_path (prog_path), m_cwd (cwd), m_session_dirs (session_dirs),
        m_global_dirs (global_dirs), m_ignore_paths (ignore_paths),
        read_line (read_line_func)
    {
    }

    bool operator () (const UString &a_file_path,
                      int a_line_number,
                      std::string &a_line)
    {
        return read_line (a_file_path, m_prog_path, m_cwd, m_session_dirs,
                          m_global_dirs, m_ignore_paths, true, a_line_number,
                          a_line);
    }
};

bool write_asm_instr (const common::Asm &a_asm,
		      ReadLine &a_read,
		      std::ostringstream &a_os);

NEMIVER_END_NAMESPACE (common)
NEMIVER_END_NAMESPACE (nemiver)

#endif // __NEMIVER_ASM_UTILS_H__
