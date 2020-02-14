# IDE suggestion

* get visual studio code from https://code.visualstudio.com/download
* install `Rust (rls)` extension
* install `Graphviz` extension to be able to view `.dot` files.
    * Click `Open Preview` (one of the symbols in the top right corner), wile looking at a graphviz file

# Manual usage of graphviz to render asm files as graphs

```bash
dot some_dot_file.dot -Tpdf -O
```

creates a pdf with the control flow graph of the asm program.
You can also choose `-Tpng` or `-Tsvg`.

# Setup

* install Rust from https://rustup.rs
* install `rustfmt` via `rustup component add rustfmt`

# Usage

* `cargo test` for building and running all tests
* `cargo fmt` for formatting the source files
* `cargo run --bin scasm tests/a.asm` for running `scasm` on a single file
* `RUST_LOG=debug cargo run --bin scasm tests/a.asm` for emitting all the `debug!` output

## License for scasm

Copyright 2019 Cosmian (cosmian.com)
Copyright 2019 COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
[https://www.apache.org/licenses/LICENSE-2.0](https://www.apache.org/licenses/LICENSE-2.0)> or the MIT license
<LICENSE-MIT or [https://opensource.org/licenses/MIT](https://opensource.org/licenses/MIT)>, at your
option. Files in the project may not be
copied, modified, or distributed except according to those terms.
