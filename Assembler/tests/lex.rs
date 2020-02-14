use pretty_assertions::assert_eq;
use scasm::asm::Body;
use scasm::lexer::*;
use scasm::span::Span;

#[test]
fn parse_check() -> std::io::Result<()> {
    for file in std::fs::read_dir("tests")? {
        let file = file?.path();
        println!("seeing: {}", file.display());
        if let Some(extension) = file.extension() {
            if extension == "asm" {
                println!("processing: {}", file.display());
                let input = std::fs::read_to_string(&file)?;
                let cx = scasm::Compiler::stderr();
                let lexed = parse(&cx, &input);
                let parsed = Body::parse(&cx, &lexed);
                let dot = std::fs::File::create(file.with_extension("dot")).unwrap();
                parsed.print_dot_file(&cx, dot).unwrap();
                let relexed = parsed.relex(&cx);
                cx.check_for_errors().unwrap();
                for (lex, relex) in lexed.iter().zip(relexed) {
                    //println!("{} \t\t {}", lex, relex);
                    assert_eq!(lex.to_string(), relex.to_string());
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
                    assert_eq!(lex.to_string(), relex.to_string());
                }
                cx.check_for_errors().unwrap();
            }
        }
    }
    Ok(())
}
