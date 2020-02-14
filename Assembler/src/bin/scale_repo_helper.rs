#![warn(clippy::all)]
#![allow(clippy::trivial_regex)]
#![deny(rust_2018_idioms)]

use std::path::PathBuf;
use std::process::Stdio;
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
                if !args.verbose {
                    cmd.stdout(Stdio::null());
                    cmd.stderr(Stdio::null());
                }
                if !cmd.status()?.success() {
                    println!("Failed to compile {}", file.display());
                    std::process::exit(1);
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
                    // Just convert between bc and asm file
                    cmd.arg("--optimization-level=0");
                    if !args.verbose {
                        cmd.stdout(Stdio::null());
                        cmd.stderr(Stdio::null());
                    }
                    if !cmd.status()?.success() {
                        println!("Failed to compile {}", file.display());
                        std::process::exit(1);
                    }
                }
            }
        }
    }
    Ok(())
}
