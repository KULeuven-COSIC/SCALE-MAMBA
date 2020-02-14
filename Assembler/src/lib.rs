#![warn(clippy::all)]
#![deny(rust_2018_idioms)]
#![allow(clippy::cognitive_complexity)]

#[macro_use]
extern crate tracing;
use tracing_subscriber::EnvFilter;

macro_rules! check_type_size {
    ($ident:ident: $ty:ty, $size:expr) => {
        #[allow(dead_code)]
        static $ident: [(); $size] = [(); std::mem::size_of::<$ty>()];
    };
}

pub mod asm;
mod compiler;
pub mod cranelift;
mod errors;
#[macro_use]
pub mod lexer;
pub mod span;
pub mod transforms;

pub mod binary;

pub use compiler::*;

pub fn init_logger() -> Result<(), String> {
    tracing::subscriber::set_global_default(
        tracing_subscriber::FmtSubscriber::builder()
            .with_env_filter(EnvFilter::from_default_env())
            .finish(),
    )
    .map_err(|e| e.to_string())?;

    tracing_log::LogTracer::init().map_err(|e| e.to_string())
}
