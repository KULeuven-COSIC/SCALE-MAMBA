use std::io::Write;

#[test]
#[allow(clippy::write_literal)]
fn generate_instruction_tex_table() -> std::io::Result<()> {
    let mut file = std::fs::File::create("Documentation/Instructions.tex")?;
    for group in scasm::binary::instructions::INSTRUCTIONS {
        writeln!(file, r"\subsubsection{{{}}}", group.name.trim())?;

        writeln!(file, "{}", r"\begin{longtable}{|l|c|p{1in}|c|p{2.27in}|c|}")?;
        writeln!(file, r"\hline")?;

        writeln!(
            file,
            r"Instr & Code & Operands & Vect & Description & Notes \\"
        )?;
        writeln!(file, r"\hline")?;

        for i in group.instructions {
            let name = i.name.replace("_", "\\_");
            write!(file, "  {} & 0x{:X} & [", name, i.opcode)?;
            let mut first = true;
            for arg in i.args {
                if first {
                    first = false;
                } else {
                    write!(file, ", ")?;
                }
                write!(file, "'{}'", arg.ty)?;
            }
            write!(file, "] & ")?;
            if i.vectorizable {
                write!(file, r"\tick ")?;
            }
            write!(file, " & {} & ", i.comment.trim())?;
            let mut flags = Vec::new();
            if !i.notes.trim().is_empty() {
                flags.push(i.notes.trim());
            }
            if i.terminator {
                flags.push("$\\ddagger$")
            } else if i.barrier {
                flags.push("$\\dagger$");
            }
            if i.thread_0_only {
                flags.push("$\\tau_0$");
            }
            if i.mem_read {
                flags.push("$\\star(r)$");
            }
            if i.mem_write {
                flags.push("$\\star(w)$");
            }
            let notes = flags.join(", ");
            write!(file, "{}", notes)?;

            writeln!(file, " \\\\")?;
        }
        writeln!(file, r"\hline")?;
        writeln!(file, "{}", r"\end{longtable}")?;
    }
    Ok(())
}
