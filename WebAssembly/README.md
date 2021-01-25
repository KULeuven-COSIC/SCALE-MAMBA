**was**m to **sca**le transpiler

## Setup with nix

First, set up your build environment by invoking

```
nix-shell ../shell.nix
```

This will open a new shell, where all the wasm tools and the appropriate rust version are available.

## Manual setup

In addition to the general scasm environment (like a Rust compiler), you also need the
`wasm-opt` and `wasm2wat` programs. These are found on most platforms in the
`binaryen` and `wabt` packages.

Alas in Ubuntu this is a problem so we used the script
  ./install-wasm-tools.sh
in this directory.

XXXX Looking for a fix on CentOS and other systems where users do not have
XXXX sudo password


## Running tests

Now, run

```
./run_on_all_demos.sh
```

to convert all the demos in `scale/demos` from Rust to scale assembly.

The resulting `.asm` file is in textual asm form, so if you want to have the binary format, you need
to invoke

```
cargo run --manifest-path ../Assembler/Cargo.toml -- --optimization-level 1 scale/demos/your_demo/scale_assembly.asm output_byte_code_file.bc
```

## Creating a new demo/test

Run

```
./create_new_test.sh you_test_name
```

which will create a fully functioning new test in `scale/demos` that will automatically be run
when you run `./run_on_all_demos.sh`.
