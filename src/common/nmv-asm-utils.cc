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

#include "nmv-asm-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (common)

void
log_asm_insns (const std::list<common::Asm> &a_asm)
{
    typedef std::list<common::Asm> Asms;
    Asms::const_iterator it = a_asm.begin ();
    if (it != a_asm.end ()) {
        LOG_DD (*it);
    }
    for (++it; it != a_asm.end (); ++it) {
        LOG_DD ("\n" << *it);
    }
}

/// Write an asm instruction to an output stream.
///
/// \param a_instr the asm instruction to write.
///
/// \param a_os the output stream to write the instruction to.
///
/// \return true upon successful completion, false otherwise.
bool
write_asm_instr (const common::AsmInstr &a_instr,
                 std::ostringstream &a_os)
{
    a_os << a_instr.address ();
    a_os << "  ";
    a_os << "<" << a_instr.function ();
    if (!a_instr.offset ().empty () && a_instr.offset () != "0")
        a_os << "+" << a_instr.offset ();
    a_os << ">:  ";
    a_os << a_instr.instruction ();

    return true;
}

/// Write an asm instruction to an output stream.
///
/// \param a_asm the asm instruction to write.
///
/// \param a_read the functor used to read source code lines in case
/// the asm to write is mixed with higher level source code.
///
/// \param a_os the output stream to write the asm to.
///
/// \return true upon sucessful completion, false otherwise.
bool
write_asm_instr (const common::Asm &a_asm,
                 ReadLine &a_read,
                 std::ostringstream &a_os)
{
    bool written = false;

    switch (a_asm.which ()) {
    case common::Asm::TYPE_PURE:
        write_asm_instr (a_asm.instr (), a_os);
        written = true;
        break;
    case common::Asm::TYPE_MIXED: {
        const common::MixedAsmInstr &instr = a_asm.mixed_instr ();
        // Ignore requests for line 0. Line 0 cannot exist as lines
        // should be starting at 1., some
        // versions of GDB seem to be referencing it for a reason.
        if (instr.line_number () == 0) {
            LOG_DD ("Skipping asm instr at line 0");
            return false;
        }
        string line;
        if (a_read (instr.file_path (), instr.line_number (), line)) {
            if (line.empty ())
                a_os << "\n";
            else {
                a_os << line; // line does not end with a '\n' char.
                written = true;
            }
        } else {
            a_os << "<src file=\""
                 << instr.file_path ()
                 << "\" line=\""
                 << instr.line_number ()
                 << "\"/>";
            written = true;
        }

        if (!instr.instrs ().empty ()) {
            list<common::AsmInstr>::const_iterator it =
                instr.instrs ().begin ();
            if (it != instr.instrs ().end ()) {
                if (written)
                    a_os << "\n";
                written = write_asm_instr (*it, a_os);
                ++it;
            }
            for (; it != instr.instrs ().end (); ++it) {
                if (written)
                    a_os << "\n";
                written = write_asm_instr (*it, a_os);
            }
        }
    }
        break;
    default:
        break;
    }
    return written;
}

NEMIVER_END_NAMESPACE (common)
NEMIVER_END_NAMESPACE (nemiver)
