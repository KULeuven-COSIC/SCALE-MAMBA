#![warn(clippy::all)]
#![deny(rust_2018_idioms)]

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

fn main() -> Result<(), Error> {
    let cx = scasm::Compiler::stderr();
    let mut parsed = cx.parse_asm("<stdin>", std::io::stdin());
    scasm::transforms::apply_default_optimization_pipeline(&cx, &mut parsed, None);
    let ir = cx.generate_ir(parsed);

    println!("{}", ir);

    cx.check_for_errors()?;
    Ok(())
}
