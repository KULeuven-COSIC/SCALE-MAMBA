// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use pretty_assertions::assert_eq;
use scasm::asm::Body;
use scasm::lexer::*;
use scasm::span::Span;
use std::convert::TryInto;

#[test]
fn parse_check() -> std::io::Result<()> {
    scasm::init_logger().unwrap();
    for file in std::fs::read_dir("tests")? {
        let file = file?.path();
        println!("seeing: {}", file.display());
        if let Some(extension) = file.extension() {
            if extension == "asm" {
                println!("processing: {}", file.display());
                let input = std::fs::read_to_string(&file)?;
                let cx = scasm::Compiler::stderr();
                let file_id = cx.files.len();
                cx.files.push(input);
                cx.file_paths.push(file.clone());
                let lexed = parse(&cx, file_id.try_into().unwrap());
                let parsed = Body::parse(&cx, &lexed);
                let dot = std::fs::File::create(file.with_extension("dot")).unwrap();
                parsed.print_dot_file(&cx, dot).unwrap();
                let relexed = parsed.relex(&cx);
                cx.check_for_errors().unwrap();
                for (lex, relex) in lexed.iter().zip(relexed) {
                    //println!("{} \t\t {}", lex, relex);
                    assert_eq!(lex.display(&cx).to_string(), relex.display(&cx).to_string());
                }
                scasm::binary::generate_bytecode(
                    &cx,
                    &lexed,
                    std::fs::File::create(file.with_extension("bc"))?,
                )?;
                let reparsed = cx.parse_bytecode(
                    file.with_extension("bc"),
                    std::fs::File::open(file.with_extension("bc"))?,
                );
                let relexed = reparsed.relex(&cx);
                cx.check_for_errors().unwrap();
                for (lex, relex) in lexed.iter().zip(relexed) {
                    //println!("{} \t\t {}", lex, relex);
                    let lex = Lexical {
                        comment: Span::DUMMY,
                        ..lex.clone()
                    };
                    assert_eq!(lex.display(&cx).to_string(), relex.display(&cx).to_string());
                }
                cx.check_for_errors().unwrap();
            }
        }
    }
    Ok(())
}
