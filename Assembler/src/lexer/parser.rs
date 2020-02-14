use super::{register, Const, Lexical, Operand, Register, Span, Spanned};
use crate::errors;
use crate::Compiler;

pub fn parse<'a>(cx: &'a Compiler, input: &'a str) -> Vec<Lexical<'a>> {
    Span::lines(input)
        .filter_map(|span| {
            // remove comments
            let (start, comment) = span.split_at_first_char('#');
            let comment = comment.unwrap_or_else(|| span.end()).trim();
            let line = start.trim();
            let (instruction, args) = line.split_at_first_char(' ');
            let args = args
                .map(|args| args.split(',').map(|arg| parse_operand(cx, arg)).collect())
                .unwrap_or_default();
            let instruction = instruction.snippet();
            if instruction.is_empty() {
                return None;
            }
            let instruction = if instruction.chars().all(|c| c.is_ascii_lowercase()) {
                instruction
            } else {
                cx.strings.push_get(instruction.to_ascii_lowercase())
            };
            Some(Lexical {
                instruction,
                args,
                comment,
                span,
            })
        })
        .collect()
}

fn parse_operand<'a>(cx: &'a Compiler, span: Span<'a>) -> Spanned<'a, Operand> {
    let span = span.trim();
    let s = span.snippet().to_ascii_lowercase();
    match &*s {
        "true" => return span.with(Operand::Value(Const::Bool(true))),
        "false" => return span.with(Operand::Value(Const::Bool(false))),
        _ => {}
    }
    let mut chars = s.chars();
    let mut id_span = span;
    let mkfn: fn(u32) -> Register = match chars.next() {
        None => {
            return cx
                .report(crate::errors::ExpectedOperand { span })
                .with(Operand::Value(Const::Int(i32::min_value())))
        }
        Some('c') => {
            id_span.snippet = &id_span.snippet[1..];
            |i| Register::Clear(register::Clear(i))
        }
        Some('s') => {
            id_span.snippet = &id_span.snippet[1..];
            let prev = chars.clone();
            match chars.next() {
                None => unimplemented!("expected id after register bank"),
                Some('r') => {
                    id_span.snippet = &id_span.snippet[1..];
                    |i| Register::SecretRegint(register::SecretRegint(i))
                }
                Some('b') => {
                    id_span.snippet = &id_span.snippet[1..];
                    |i| Register::SecretBit(register::SecretBit(i))
                }
                Some(_) => {
                    chars = prev;
                    |i| Register::Secret(register::Secret(i))
                }
            }
        }
        Some('r') => {
            id_span.snippet = &id_span.snippet[1..];
            |i| Register::Regint(register::Regint(i))
        }
        Some(_) => {
            return s
                .parse()
                .map(|i| span.with(i))
                .unwrap_or_else(|_| {
                    cx.report(errors::ExpectedGot {
                        got: span.with(s),
                        expected: "integer",
                    })
                    .with(0)
                })
                .map(Const::Int)
                .map(Operand::Value)
        }
    };
    chars
        .as_str()
        .parse()
        .map(|c| span.with(c))
        .unwrap_or_else(|err| {
            cx.report(errors::InvalidRegisterId { span: id_span, err })
                .with(u32::max_value())
        })
        .map(|id| Operand::Register(mkfn(id)))
}
