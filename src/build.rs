// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use std::path::PathBuf;

use eyre::Context;

fn main() -> eyre::Result<()> {
    /////////////////////////////////////////////////
    // This speeds up builds a lot by not rebuilding
    // if all the input files are the same.
    /////////////////////////////////////////////////
    if std::process::Command::new("sccache")
        .arg("--version")
        .status()
        .is_ok()
    {
        eprintln!("preventing rebuilds via sccache");
        std::env::set_var("CC", "sccache cc");
        std::env::set_var("CXX", "sccache c++");
    } else {
        eprintln!("consider `cargo install sccache` to speed up future builds");
    }

    let mut build = cc::Build::new();

    build
        .cpp(true) // Use C++ instead of C compiler
        .pic(true) // Position independent code
        .opt_level(3) // Always optimize the C++, even if the Rust code is built in debug mode
        .define("MAX_MOD_SZ", "7")
        .define("MAX_GFP_SZ", "3")
        .define("USE_RUST", "1")
        .flag("-std=c++14")
        .flag("-pedantic")
        .flag("-Wimplicit-fallthrough=1") // any comment will silence the warning // BGR: fails with GCC 4.5
        .flag("-Wno-deprecated-copy") // FIXME: we should fix the problems in the source instead of silencing the warning
        .flag("-Wno-sign-compare") // Hide warnings about signed ints getting compared with unsigned ints
        .flag("-Wno-return-type") // Hide warnings about missing return types
        .flag("-Wno-non-template-friend") // Hide warnings about non-template friend function declarations
        .flag("-pthread")
        .flag("-maes")
        .flag("-mpclmul")
        .flag("-msse4.1")
        .flag("-mavx")
        .flag("-march=native")
        .flag("-funroll-loops")
        .flag("-fverbose-asm");

    // Pick up all `*.cpp` files as input to the compiler
    let files = glob::glob("**/*.cpp")?.collect::<Result<Vec<PathBuf>, _>>()?;
    // Pick up all `*.h` files
    let headers = glob::glob("**/*.h")?.collect::<Result<Vec<PathBuf>, _>>()?;
    // Put all `*.cpp` and `*.h` files into the list of files that cargo tracks.
    for file in files.iter().chain(headers.iter()) {
        println!("cargo:rerun-if-changed={}", file.display());
    }
    // Add all directories in this folder to the include paths.
    let includes = glob::glob("*")?
        .filter(|entry| entry.as_ref().map_or(true, |p| p.is_dir()))
        .collect::<Result<Vec<PathBuf>, _>>()?;
    build
        .includes(includes)
        .include(".") // Also add the current directory to include paths, as most imports are relative to this dir.
        .files(files)
        .compile("scale-mamba"); // The output `.a` file is named `scale-mamba.a`

    // Pick up some libraries via pkg_config
    pkg_config::Config::new()
        .atleast_version("1.1.0")
        .probe("openssl")?;
    pkg_config::Config::new()
        .atleast_version("1.1.0")
        .probe("libcrypto")?;

    // Some libraries do not have pkg-config files and need to be found manually
    // pkg_config::Config::new()
    //     .atleast_version("8.2.0")
    //     .probe("libcrypto")?;
    println!("cargo:rerun-if-env-changed=LIBCRYPTO_PATH");
    println!(
        "cargo:rustc-link-search=native={}",
        std::env::var("LIBCRYPTO_PATH")
            .context("cannot find `cryptopp`, please specify the `LIBCRYPTO_PATH` env var")?
    );

    // waiting for https://github.com/wbhart/mpir/issues/288
    // pkg_config::Config::new()
    //     .atleast_version("3.0.0")
    //     .probe("mpir")?;
    println!("cargo:rerun-if-env-changed=LIBMPIR_PATH");
    println!(
        "cargo:rustc-link-search=native={}",
        std::env::var("LIBMPIR_PATH")
            .context("cannot find `cryptopp`, please specify the `LIBCRYPTO_PATH` env var")?
    );
    println!("cargo:rustc-link-lib=dylib=mpirxx");
    println!("cargo:rustc-link-lib=dylib=mpir");
    println!("cargo:rustc-link-lib=static=cryptopp");

    Ok(())
}
