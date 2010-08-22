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
// Author: Dodji Seketeli

#ifndef __NEMIVER_ASM_UTILS_H__
#define __NEMIVER_ASM_UTILS_H__

#include "nmv-i-debugger.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

template<class Stream>
Stream&
operator<< (Stream &a_out, const common::AsmInstr &a_instr)
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
operator<< (Stream &a_out, const common::MixedAsmInstr &a_instr)
{
    a_out << "<asm-mixed-instr>\n"
          << " <line>" << a_instr.line_number () << "</line>\n"
          << " <path>" << a_instr.file_path ()   << "</path>\n";

    list<common::AsmInstr>::const_iterator it;
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
operator<< (Stream &a_out, const common::Asm &a_asm)
{
    switch (a_asm.which ()) {
        case common::Asm::TYPE_PURE:
            a_out << a_asm.instr ();
            break;
        case common::Asm::TYPE_MIXED:
            a_out << a_asm.mixed_instr ();
            break;
        default:
            THROW ("reached unreachable");
    }
    return a_out;
}

NEMIVER_END_NAMESPACE (nemiver)
#endif // __NEMIVER_ASM_UTILS_H__ 
