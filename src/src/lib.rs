// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use std::os::raw::{c_char, c_int, c_uchar, c_uint, c_void};

extern "C" {
    #[allow(improper_ctypes)]
    pub fn run_scale_from_rust(
        portnumbase: c_uint,
        memtype: *const c_char,
        verbose: c_int,
        fhe_factories: c_int,
        pns: *const c_uint,
        pns_n: c_uint,
        min: *const c_int,
        max: *const c_int,
        ot_disable: c_uchar,
        max_i: c_uint,
        playernum: c_uint,
        progname: *const c_char,
        io_constructor: unsafe extern "C" fn(*const c_void) -> *const c_void,
        io_handle: &mut ScaleMambaHandle,
    ) -> c_int;

    pub fn create_input_output_base_rust(handle: *const c_void) -> *const c_void;

    pub fn init_certs_rust(cin: *const c_char) -> c_int;
    pub fn init_secret_sharing_rust(cin: *const c_char) -> c_int;
    pub fn init_conversion_rust() -> c_int;

    pub fn init_certs_cin() -> c_int;
    pub fn init_secret_sharing_cin() -> c_int;
}

/// The source of the LSSS to GC converter to be passed to `init_secret_sharing` in order
/// to trigger the compilation of a properly sized circuit.
pub const LSSS_TO_GC: &str = include_str!("../../Circuits/Bristol/LSSS_to_GC.txt");

macro_rules! circuit {
    {$e:expr} => { ($e, include_str!(concat!("../../Circuits/Bristol/", $e))) };
}

/// The filenames and sources for the bristol circuits
pub const CIRCUITS: &[(&str, &str)] = &[
    circuit!("adder64.txt"),
    circuit!("aes_128.txt"),
    circuit!("aes_192.txt"),
    circuit!("aes_256.txt"),
    circuit!("AES-non-expanded.txt"),
    circuit!("divide64.txt"),
    circuit!("Keccak_f.txt"),
    circuit!("ModAdd512.txt"),
    circuit!("mult2_64.txt"),
    circuit!("mult64.txt"),
    circuit!("neg64.txt"),
    circuit!("sha256.txt"),
    circuit!("sha512.txt"),
    circuit!("sub64.txt"),
    circuit!("udivide64.txt"),
    circuit!("zero_equal.txt"),
];

mod scale_mamba_handler;

pub use scale_mamba_handler::{Result, ScaleMambaHandle, ScaleMambaHandler};
