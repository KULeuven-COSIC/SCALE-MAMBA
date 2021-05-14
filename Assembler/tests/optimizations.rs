// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![allow(clippy::cognitive_complexity)]

use std::fs::File;
use std::io::Write;

#[macro_use]
extern crate tracing;

#[test]
fn optimizations() -> std::io::Result<()> {
    scasm::init_logger().unwrap();
    for file in std::fs::read_dir("tests/optimizations")? {
        let file = file?.path();
        let file_path = file.display();
        let span = info_span!("optimizing", %file_path);
        let _enter = span.enter();
        debug!("seen");
        if let Some(extension) = file.extension() {
            if extension == "asm" {
                if let Ok(val) = std::env::var("OPTIMIZATION_TESTS") {
                    if !file.file_name().unwrap().to_str().unwrap().contains(&val) {
                        continue;
                    }
                }
                info!("processing");
                let helper = scasm::Helper::default();
                let cx = scasm::Compiler::new(Box::new(helper.clone()), false);
                let mut parsed = cx.parse_asm(&file, std::fs::File::open(&file)?);
                scasm::transforms::apply_default_optimization_pipeline(&cx, &mut parsed, None);
                let lexed = parsed.relex(&cx);

                let optimized = file.with_extension("optimized");
                let mut optimized = File::create(optimized)?;
                for lex in &lexed {
                    writeln!(optimized, "{}", lex.display(&cx))?;
                }

                if cx.check_for_errors().is_ok() {
                    info!("generating bytecode file");
                    // everything that compiles without errors gets compiled to machine code
                    let file = File::create(file.with_extension("bc"))?;
                    scasm::binary::generate_bytecode(&cx, &lexed, file)?;
                } else if file.with_extension("bc").exists() {
                    warn!("removing bc file");
                    let _ = std::fs::remove_file(file.with_extension("bc"));
                }

                drop(cx);

                let stderr = file.with_extension("stderr");
                let expected = std::fs::read_to_string(&stderr).unwrap_or_default();
                let output = helper.into_inner();
                if output.trim().is_empty() {
                    warn!("removing stderr file");
                    let _ = std::fs::remove_file(stderr);
                } else if expected.trim() != output.trim() {
                    std::fs::write(stderr, output.trim()).unwrap();
                }
            }
        }
    }
    Ok(())
}
