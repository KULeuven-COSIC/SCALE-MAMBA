// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![warn(clippy::all)]
#![allow(clippy::trivial_regex)]
#![deny(rust_2018_idioms)]

use parse_display::{Display, FromStr};
use scasm::asm::Body;
use scasm::Compiler;
use std::io::Read;
use std::io::Write;
use std::path::PathBuf;
use structopt::StructOpt;

#[derive(Debug)]
enum Error {
    Io(std::io::Error),
    AlreadyReported,
    Raw(String),
}

impl From<std::io::Error> for Error {
    fn from(err: std::io::Error) -> Self {
        Error::Io(err)
    }
}

impl From<scasm::ErrorReported> for Error {
    fn from(_: scasm::ErrorReported) -> Self {
        Error::AlreadyReported
    }
}

impl From<String> for Error {
    fn from(s: String) -> Self {
        Error::Raw(s)
    }
}

impl<'a> From<&'a str> for Error {
    fn from(s: &'a str) -> Self {
        Error::Raw(s.to_string())
    }
}

#[derive(StructOpt)]
struct Args {
    /// Do not show warnings, only errors.
    #[structopt(long)]
    hide_warnings: bool,
    /// Optimization level, level 0 means no optimizations.
    #[structopt(short = "O", long, default_value = "3")]
    optimization_level: u8,
    /// Optimizations to run. Overrides `--optimization-level` and just sets the list of optimizations that are run.
    /// If empty, dumps the list of optimizations available
    #[structopt(long)]
    optimizations: Option<Vec<String>>,
    /// The input format, either `assembly` or `bytecode`. If not specified, it is automatically chosen from the input file extension.
    #[structopt(long)]
    input_format: Option<InputFormat>,
    /// The output format, either `assembly`, `graphviz` or `bytecode`. If not specified, it is automatically chosen from the output file extension.
    #[structopt(long)]
    output_format: Option<OutputFormat>,
    /// Dump the asm after each optimization step to the given directory.
    #[structopt(long, parse(from_os_str))]
    dump_optimizations: Option<PathBuf>,
    /// The path to the file to process.
    /// The input format is figured out from the file extension and can be either
    /// `.asm` or `.bc`
    /// If this is missing, the input is read from `stdin` and optimized assembly is output to `stdout`
    #[structopt(parse(from_os_str))]
    input: Option<PathBuf>,
    /// The path to write the output to.
    /// The output format is figured out from the extension and can be one of
    /// * `.asm` (scasm assembly)
    /// * `.bc` (bytecode)
    /// * `.dot` (graphviz dot file showing the control flow graph of the input)
    ///
    /// If this is missing, the output is written to `stdout` in scale assembly format
    #[structopt(parse(from_os_str))]
    output: Option<PathBuf>,
}

#[derive(Display, FromStr, PartialEq, Debug)]
#[display(style = "snake_case")]
enum InputFormat {
    Assembly,
    Bytecode,
}

#[derive(Display, FromStr, PartialEq, Debug)]
#[display(style = "snake_case")]
enum OutputFormat {
    Assembly,
    Bytecode,
    Graphviz,
}

#[paw::main]
fn main(args: Args) -> Result<(), Error> {
    scasm::init_logger()?;
    if let Some(opts) = &args.optimizations {
        if opts.is_empty() {
            scasm::transforms::list_optimizations();
            return Ok(());
        }
    }

    let mut cx = scasm::Compiler::stderr();
    cx.show_warnings = !args.hide_warnings;
    cx.optimization_level = args.optimization_level;

    let input_format = match args.input_format {
        Some(f) => f,
        None => match &args.input {
            Some(file) => {
                let ext = file
                    .extension()
                    .ok_or("cannot write to file without file extension")?;
                match ext.to_str() {
                    Some("asm") => InputFormat::Assembly,
                    Some("bc") => InputFormat::Bytecode,
                    _ => return Err(format!("unknown file extension {:?}", ext).into()),
                }
            }
            None => InputFormat::Assembly,
        },
    };

    let mut parsed = if let Some(file) = args.input {
        let f = std::fs::File::open(&file)?;
        read_input(file, f, &cx, input_format)
    } else {
        read_input(
            PathBuf::from("<stdin>"),
            std::io::stdin(),
            &cx,
            input_format,
        )
    };

    cx.check_for_errors()?;

    if let Some(opts) = args.optimizations {
        scasm::transforms::run_optimizations(&cx, &mut parsed, args.dump_optimizations, &opts)
    } else {
        scasm::transforms::apply_default_optimization_pipeline(
            &cx,
            &mut parsed,
            args.dump_optimizations,
        );
    }

    cx.check_for_errors()?;

    let output_format = match args.output_format {
        Some(of) => of,
        None => match &args.output {
            Some(file) => {
                let ext = file
                    .extension()
                    .ok_or("cannot write to file without file extension")?;
                match ext.to_str() {
                    Some("asm") => OutputFormat::Assembly,
                    Some("bc") => OutputFormat::Bytecode,
                    Some("dot") => OutputFormat::Graphviz,
                    _ => return Err(format!("unknown file extension {:?}", ext).into()),
                }
            }
            None => OutputFormat::Assembly,
        },
    };

    if let Some(file) = args.output {
        write_output(std::fs::File::create(file)?, &cx, parsed, output_format)?;
    } else {
        write_output(std::io::stdout(), &cx, parsed, output_format)?;
    }

    cx.check_for_errors()?;
    Ok(())
}

fn read_input(source: PathBuf, f: impl Read, cx: &Compiler, input_format: InputFormat) -> Body<'_> {
    match input_format {
        InputFormat::Assembly => cx.parse_asm(source, f),
        InputFormat::Bytecode => cx.parse_bytecode(source, f),
    }
}

fn write_output<'a>(
    mut f: impl Write,
    cx: &'a Compiler,
    parsed: Body<'a>,
    output_format: OutputFormat,
) -> Result<(), Error> {
    match output_format {
        OutputFormat::Assembly => {
            let relexed = parsed.relex(&cx);
            for lex in relexed {
                writeln!(f, "{}", lex.display(cx))?;
            }
        }
        OutputFormat::Graphviz => parsed.print_dot_file(cx, f)?,
        OutputFormat::Bytecode => {
            let relexed = parsed.relex(&cx);
            scasm::binary::generate_bytecode(&cx, &relexed, f)?
        }
    }
    Ok(())
}
