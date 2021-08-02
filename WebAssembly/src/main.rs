// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![warn(clippy::all)]
#![allow(clippy::trivial_regex)]
#![deny(rust_2018_idioms)]

use wasca::Error;

#[paw::main]
fn main(args: wasca::Args) -> Result<(), Error> {
    scasm::init_logger()?;
    wasca::main(args).map_err(|s| s.to_string().into())
}
