// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#[test]
fn check_broken_asm() -> std::io::Result<()> {
    for file in std::fs::read_dir("tests/broken_asm")? {
        let file = file?.path();
        println!("seeing: {}", file.display());
        if let Some(extension) = file.extension() {
            if extension == "asm" {
                println!("processing: {}", file.display());
                let helper = scasm::Helper::default();
                let cx = scasm::Compiler::new(Box::new(helper.clone()), false);
                let mut parsed = cx.parse_asm(&file, std::fs::File::open(&file)?);
                if cx.check_for_errors().is_ok() {
                    scasm::transforms::apply_default_optimization_pipeline(&cx, &mut parsed, None);

                    if cx.check_for_errors().is_ok() {
                        panic!(
                            "{} compiled successfully but was expected to fail",
                            file.display()
                        );
                    }
                }
                drop(cx);
                let output = helper.into_inner();
                let stderr = file.with_extension("stderr");
                let expected = std::fs::read_to_string(&stderr).unwrap_or_default();
                if expected.trim() != output.trim() {
                    std::fs::write(stderr, output.trim()).unwrap();
                }
            }
        }
    }
    Ok(())
}
