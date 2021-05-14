// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![warn(clippy::all)]
#![deny(rust_2018_idioms)]

#[macro_use]
extern crate tracing;

use scasm::lexer::Lexical;
use scasm::span::Span;
use std::convert::TryInto;

#[derive(Debug)]
enum Error {
    Io(std::io::Error),
    ErrorReported,
}

impl From<std::io::Error> for Error {
    fn from(err: std::io::Error) -> Self {
        Error::Io(err)
    }
}

impl From<scasm::ErrorReported> for Error {
    fn from(_: scasm::ErrorReported) -> Self {
        Error::ErrorReported
    }
}

#[allow(clippy::cognitive_complexity)]
fn main() -> std::io::Result<()> {
    scasm::init_logger().unwrap();

    for file in std::fs::read_dir("tests/scasm-tests")? {
        let file = file?.path();
        print!("seeing: {} ... ", file.display());
        if file == std::path::Path::new("tests/scasm-tests/scasm-test_math-0.asm")
            && std::env::var("SKIP_HUGE_MATH_TEST").is_ok()
        {
            println!("skipping: {}", file.display());
            continue;
        }
        if let Some(extension) = file.extension() {
            if extension == "asm" {
                println!("procssing");
                let mut cx = scasm::Compiler::stderr();
                // these are just noise here
                cx.show_warnings = false;
                let mut parsed = cx.parse_asm(&file, std::fs::File::open(&file).unwrap());
                let relexed = parsed.relex(&cx);
                if cx.check_for_errors().is_ok() {
                    let file_id = cx.file_paths.iter().position(|f| *f == file).unwrap();
                    let lexed = cx.lex(file_id.try_into().unwrap());
                    for (lex, relex) in lexed.iter().zip(relexed) {
                        assert_eq!(lex.display(&cx).to_string(), relex.display(&cx).to_string());
                    }

                    info!("optimize");
                    scasm::transforms::apply_default_optimization_pipeline(&cx, &mut parsed, None);

                    info!("generate bytecode");

                    scasm::binary::generate_bytecode(
                        &cx,
                        &lexed,
                        std::fs::File::create(file.with_extension("bc"))?,
                    )?;
                    info!("parse bytecode");
                    let reparsed = cx.parse_bytecode(
                        file.with_extension("bc"),
                        std::fs::File::open(file.with_extension("bc"))?,
                    );
                    info!("relex");
                    let relexed = reparsed.relex(&cx);
                    info!("cmp");
                    for (lex, relex) in lexed.iter().zip(relexed) {
                        trace!("{} \t\t {}", lex.display(&cx), relex.display(&cx));
                        let lex = Lexical {
                            comment: Span::DUMMY,
                            ..lex.clone()
                        };
                        assert_eq!(lex.instruction, relex.instruction);
                        assert_eq!(lex.args.len(), relex.args.len());
                        for (a, b) in lex.args.iter().zip(relex.args.iter()) {
                            if a.elem != b.elem {
                                panic!("{} != {}", lex.display(&cx), relex.display(&cx));
                            }
                        }
                    }
                }
                let _ = cx.check_for_errors();
            } else {
                println!("skipping");
            }
        }
    }
    Ok(())
}
