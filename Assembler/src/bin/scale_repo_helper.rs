// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![warn(clippy::all)]
#![allow(clippy::trivial_regex)]
#![deny(rust_2018_idioms)]

use std::io::Write;
use std::path::PathBuf;
use structopt::StructOpt;

#[derive(StructOpt)]
struct Args {
    /// Print any information to the user. Otherwise the command is entirely silent.
    #[structopt(long)]
    verbose: bool,
    /// The directory to read all `.asm` files from and compile them to `.bc`
    #[structopt(parse(from_os_str))]
    directory: PathBuf,
    /// Also generate `.asm-opt` files showing the optimized assembly
    #[structopt(long)]
    show_optimization: bool,
    /// Flags directly forwarded to the `scasm` compiler. Use `./scasm --verbose Programs/some_dir_with_bc_file -- --help` to show all the available commands.
    #[structopt(raw(true))]
    rest: Vec<String>,
}

#[paw::main]
fn main(args: Args) -> std::io::Result<()> {
    if args.verbose {
        println!("reading all `.asm` files in {}", args.directory.display());
    }
    let manifest = std::env::current_exe()?
        .parent()
        .unwrap()
        .parent()
        .unwrap()
        .parent()
        .unwrap()
        .join("Cargo.toml");
    for file in std::fs::read_dir(args.directory)? {
        let file = file?.path();
        if let Some(extension) = file.extension() {
            if extension == "asm" {
                if args.verbose {
                    println!("processing: {}... ", file.display());
                }
                let mut cmd = escargot::CargoBuild::new()
                    .bin("scasm")
                    .current_release()
                    .manifest_path(&manifest)
                    .run()
                    .unwrap()
                    .command();

                cmd.arg(&file);
                cmd.arg(file.with_extension("bc"));
                cmd.args(&args.rest);
                let output = cmd.output()?;
                if !output.status.success() {
                    println!("Failed to compile {}", file.display());
                    std::io::stdout().write_all(&output.stdout)?;
                    std::io::stdout().flush()?;
                    std::io::stderr().write_all(&output.stderr)?;
                    std::io::stderr().flush()?;
                    std::process::exit(1);
                } else {
                    std::io::stdout().write_all(&output.stdout)?;
                    std::io::stdout().flush()?;
                    std::io::stderr().write_all(&output.stderr)?;
                    std::io::stderr().flush()?;
                }
                if args.show_optimization {
                    let mut cmd = escargot::CargoBuild::new()
                        .bin("scasm")
                        .current_release()
                        .manifest_path(&manifest)
                        .run()
                        .unwrap()
                        .command();
                    cmd.arg(file.with_extension("bc"));
                    cmd.arg(file.with_extension("asm-opt"));
                    cmd.arg("--output-format=assembly");
                    // Just convert between bc and asm file
                    cmd.arg("--optimization-level=0");
                    let output = cmd.output()?;
                    if !output.status.success() {
                        println!("Failed to compile {}", file.display());
                        std::io::stdout().write_all(&output.stdout)?;
                        std::io::stdout().flush()?;
                        std::io::stderr().write_all(&output.stderr)?;
                        std::io::stderr().flush()?;
                        std::process::exit(1);
                    }
                }
            }
        }
    }
    Ok(())
}
