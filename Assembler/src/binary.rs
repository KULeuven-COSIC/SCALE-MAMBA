// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::binary::instructions::name2instr;
use crate::compiler::Compiler;
use crate::errors::Io;
use crate::errors::NotVectorizable;
use crate::errors::UnimplementedInstruction;
use crate::lexer::{Const, Lexical, Operand, Register};
use crate::span::Span;
use crate::span::Spanned;
use instructions::ArgTy;
use std::collections::HashMap;
use std::io::Read;

pub use documentation as instructions;

pub fn generate_bytecode<'a>(
    cx: &'a Compiler,
    asm: &[Lexical<'a>],
    mut dest: impl std::io::Write,
) -> std::io::Result<()> {
    for lex in asm {
        let get_instr = |name: &str, span| match name2instr(name) {
            Some(val) => val,
            None => {
                cx.report(UnimplementedInstruction { span, name });
                name2instr("crash").unwrap()
            }
        };
        let (opcode, args) = if lex.instruction.starts_with('v') {
            let instr = &get_instr(&lex.instruction[1..], lex.span);
            assert!(instr.vectorizable);
            let (v, args) = lex.args.split_first().unwrap();
            let v: u32 = cx.checked_from(*v).elem;
            // magic number 9 taken from SCALE documentation
            (instr.opcode | v << 9, args)
        } else {
            (get_instr(lex.instruction, lex.span).opcode, &lex.args[..])
        };
        dest.write_all(&opcode.to_be_bytes())?;
        for arg in args {
            // The instruction encodes what the argument means, so we just convert
            // everything to u32
            let i = match arg.elem {
                Operand::Value(Const::Int(i)) => i as u32,
                Operand::Value(Const::Bool(b)) => b.into(),
                Operand::Register(Register::Secret(r)) => r.0,
                Operand::Register(Register::SecretRegint(r)) => r.0,
                Operand::Register(Register::Clear(r)) => r.0,
                Operand::Register(Register::Regint(r)) => r.0,
                Operand::Register(Register::SecretBit(r)) => r.0,
            };
            dest.write_all(&i.to_be_bytes())?;
        }
    }
    Ok(())
}

pub fn parse<'a>(cx: &'a Compiler, input: &'a [u8]) -> Vec<Lexical<'a>> {
    let op2instr: HashMap<_, _> = instructions::INSTRUCTIONS
        .iter()
        .flat_map(|group| group.instructions)
        .map(|i| (i.opcode, i))
        .collect();

    let mut current = input;
    let mut lexicals = Vec::new();
    let mut read = |eof_ok| {
        if eof_ok && current.is_empty() {
            return Err(());
        }
        let mut bytes = [0; 4];
        match current.read_exact(&mut bytes) {
            Ok(()) => Ok(u32::from_be_bytes(bytes)),
            Err(err) => {
                cx.report(Io { err });
                Err(())
            }
        }
    };
    loop {
        let instruction = match read(true) {
            Ok(instruction) => instruction,
            Err(()) => return lexicals,
        };
        let opcode = instruction & 0b0000_0001_1111_1111;
        let v = instruction >> 9;
        let instruction = match op2instr.get(&opcode) {
            Some(instr) => instr,
            None => {
                panic!("{} is not a valid opcode", opcode);
            }
        };
        let mut args = Vec::new();
        if v > 1 {
            if !instruction.vectorizable {
                cx.report(NotVectorizable {
                    instr: instruction.name,
                    v,
                });
            }
            args.push(Operand::from(v as i32).into());
        }
        let arg_pos = |name: &str| -> (usize, &'static ArgTy) {
            let pos = instruction
                .args
                .iter()
                .position(|i| i.name == name)
                .unwrap();
            let adjusted_pos = if v > 1 { pos + 1 } else { pos };
            (adjusted_pos, &instruction.args[pos].ty)
        };
        for arg in instruction.args {
            fn parse_arg<'a>(
                cx: &'a Compiler,
                ty: &'static ArgTy,
                read: &mut impl FnMut(bool) -> Result<u32, ()>,
                args: &mut Vec<Spanned<Operand>>,
                arg_pos: impl (Fn(&str) -> (usize, &'static ArgTy)) + Copy,
            ) -> Result<(), ()> {
                match ty {
                    // FIXME: treat unsigned and signed types differently
                    ArgTy::Int { .. } => {
                        let i = read(false)?;
                        args.push(Operand::from(i as i32).into())
                    }
                    ArgTy::List {
                        element_type,
                        len_arg,
                    } => {
                        let (pos, ty) = arg_pos(len_arg);
                        let n: Spanned<i32> = args[pos].require(cx);
                        let n = match ty {
                            ArgTy::Int {
                                signed: false,
                                offset,
                            } => n.elem - offset,
                            ty => panic!("invalid array length type {:?}", ty),
                        };
                        for _ in 0..n {
                            parse_arg(cx, element_type, read, args, arg_pos)?;
                        }
                    }
                    ArgTy::Register(kind, _) => {
                        let i = read(false)?;
                        args.push(Operand::from(Register::new(*kind, i)).into())
                    }
                }
                Ok(())
            }
            if parse_arg(cx, &arg.ty, &mut read, &mut args, arg_pos).is_err() {
                return lexicals;
            }
        }
        let instruction = instruction.name.to_ascii_lowercase();
        let instruction = if v > 1 {
            format!("v{}", instruction)
        } else {
            instruction
        };
        let instruction = cx.strings.push_get(instruction);
        let lexical = Lexical {
            instruction,
            args,
            comment: Span::DUMMY,
            span: Span::DUMMY,
        };
        lexicals.push(lexical);
    }
}
