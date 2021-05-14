// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use scale_binaries::*;
use std::{
    ffi::CString,
    os::raw::{c_int, c_uint},
    ptr,
};
use structopt::StructOpt;

#[derive(StructOpt, Debug)]
struct Args {
    /// Port number base to attempt to start connections from
    #[structopt(default_value = "5000", long)]
    portnumbase: c_uint,

    /// List of portnums, one per player.
    /// Must be the same on each machine.
    /// This option overides any defined portnumbase.
    #[structopt(long)]
    portnums: Option<Vec<c_uint>>,

    /// Set the verbose level.
    /// If positive the higher the value the more stuff printed for offline.
    /// If negative we print verbose for the online phase.
    #[structopt(default_value = "0", short = "v", long)]
    verbose: i32,

    /// Disable OT production. Will be faster/less memory, but some operations will not work.
    #[structopt(long = "dOT")]
    disable_ot_production: bool,

    /// Where to obtain memory, old|empty.
    /// old: reuse previous memory in Memory-P<i>
    /// empty: create new empty memory
    #[structopt(default_value = "empty", short = "m", long = "memory")]
    memtype: String,

    /// Do not start online phase until we have
    /// m triples, s squares and b bits.
    /// Must be the same on each machine.
    #[structopt(long = "min")]
    minimums: Option<Vec<c_int>>,

    /// Stop the offline phase when m triples, s squares and b bits have been produced.
    /// Zero argument means infinity here.
    /// Must be the same on each machine.
    #[structopt(long = "max")]
    maximums: Option<Vec<c_int>>,

    /// Similar to --max but for the input queue only
    #[structopt(default_value, long = "maxI")]
    max_i: c_uint,

    /// Number of FHE Factories we run in parallel
    #[structopt(default_value = "2", long = "fhefactories", short = "f")]
    fhe_factories: c_int,

    /// ID of the local player
    playernum: c_uint,

    /// Path to the program directory of the program to be executed
    progname: String,
}

#[paw::main]
fn main(args: Args) -> eyre::Result<()> {
    println!("{:#?}", args);
    let Args {
        portnumbase,
        portnums,
        verbose,
        disable_ot_production,
        memtype,
        minimums,
        maximums,
        max_i,
        fhe_factories,
        playernum,
        progname,
    } = args;

    let memtype = CString::new(memtype).unwrap();
    let progname_c = CString::new(progname.as_bytes()).unwrap();
    if let Some(minimums) = &minimums {
        eyre::ensure!(minimums.len() == 3, "invalid number of minimums");
    }
    if let Some(maximums) = &maximums {
        eyre::ensure!(maximums.len() == 3, "invalid number of maximums");
    }

    let mut handle = ScaleMambaHandle {
        handler: &mut SimpleIo(progname),
    };

    let exit_code = unsafe {
        run_scale_from_rust(
            portnumbase,
            memtype.as_ptr(),
            verbose,
            fhe_factories,
            portnums.as_ref().map_or(ptr::null(), |pns| pns.as_ptr()),
            portnums.as_ref().map_or(0, |pns| pns.len() as _),
            minimums
                .as_ref()
                .map_or([0; 3].as_ptr(), |min| min.as_ptr()),
            maximums
                .as_ref()
                .map_or([0; 3].as_ptr(), |max| max.as_ptr()),
            disable_ot_production as _,
            max_i,
            playernum,
            progname_c.as_ptr(),
            create_input_output_base_rust,
            &mut handle,
        )
    };

    eyre::ensure!(exit_code == 0, "the scale process crashed");

    Ok(())
}

struct SimpleIo(String);

impl ScaleMambaHandler for SimpleIo {
    fn close(&mut self) -> Result<()> {
        println!("Shutting down");
        Ok(())
    }

    fn open_channel(&self, channel: u32) -> Result<()> {
        println!("Opening channel {}", channel);
        Ok(())
    }

    fn close_channel(&self, channel: u32) -> Result<()> {
        println!("Closing channel {}", channel);
        Ok(())
    }

    fn private_input_gfp(&mut self, channel: u32) -> Result<Vec<u8>> {
        print!("Enter value on channel {} : ", channel);
        let mut s = String::new();
        std::io::stdin().read_line(&mut s).unwrap();
        Ok(s.into_bytes())
    }

    fn public_input_gfp(&self, channel: u32) -> Result<Vec<u8>> {
        print!("Enter value on channel {} : ", channel);
        let mut s = String::new();
        std::io::stdin().read_line(&mut s).unwrap();
        Ok(s.into_bytes())
    }

    fn public_input_int(&self, channel: u32) -> Result<i64> {
        print!("Enter value on channel channel {} : ", channel);
        let mut s = String::new();
        std::io::stdin().read_line(&mut s).unwrap();
        Ok(s.parse().unwrap())
    }

    fn private_output_gfp(&mut self, channel: u32, value: &[u8]) -> Result<()> {
        println!(
            "Output channel {} : {:?}",
            channel,
            std::str::from_utf8(value),
        );
        Ok(())
    }

    fn public_output_gfp(&self, channel: u32, value: &[u8]) -> Result<()> {
        println!(
            "Output channel {} : {:?}",
            channel,
            std::str::from_utf8(value),
        );
        Ok(())
    }

    fn public_output_int(&mut self, channel: u32, value: i64) -> Result<()> {
        println!("Output channel {} : {} = {:x}", channel, value, value,);
        Err(String::from("unimplemented"))
    }

    fn input_share(&self, channel: u32) -> Result<Vec<u8>> {
        println!(
            "Scale Mamba calling public input share channel: {}",
            channel
        );
        Err(String::from("unimplemented"))
    }

    fn output_share(&self, channel: u32, value: &[u8]) -> Result<()> {
        println!(
            "Scale Mamba calling output share channel: {}, value: {:?}",
            channel, value
        );
        Err(String::from("unimplemented"))
    }

    fn trigger(&mut self) -> Result<Vec<u8>> {
        println!("Restart requested: Enter a number to proceed");
        std::io::stdin().read_line(&mut String::new()).unwrap();
        Ok(self.0.clone().into_bytes())
    }

    fn debug_output(&mut self, text: &[u8]) {
        for &c in text {
            print!("{}", c as char);
        }
    }
}
