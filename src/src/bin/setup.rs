// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use eyre::Context;
use scale_binaries::*;
use std::{env, io::Write};

fn main() -> eyre::Result<()> {
    let ans: u8 = match env::args().len() {
        1 => {
            println!("What do you want to set up?");
            println!("\t1) Certs");
            println!("\t2) Secret Sharing");
            println!("\t3) Conversion circuit for LSSS<->GC computations");
            println!("\t4) All three");
            loop {
                println!("Enter a number (1-4)..");
                std::io::stdout().flush()?;
                let mut buf = String::new();
                std::io::stdin().read_line(&mut buf)?;
                let ans = buf.trim().parse::<u8>()?;
                if ans < 1 || ans > 4 {
                    continue;
                }
                break ans;
            }
        }
        2 => {
            let n = env::args()
                .nth(1)
                .unwrap()
                .parse()
                .context("parsing first argument as an integer")?;
            eyre::ensure!(n <= 4, "argument must be in the range [0, 4]");
            n
        }
        n => {
            eyre::bail!("`setup` takes zero or one arguments, but was given {}", n);
        }
    };

    if ans == 1 || ans == 4 {
        unsafe {
            init_certs_cin();
        }
    }
    if ans == 2 || ans == 4 {
        unsafe {
            init_secret_sharing_cin();
        }
    }
    if ans == 3 || ans == 4 {
        unsafe {
            init_conversion_rust();
        }
    }
    Ok(())
}
