// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use std::ffi::CStr;
use std::os::raw::{c_char, c_int, c_long, c_uint};

use tracing::{debug, error, trace};

pub type Result<T> = std::result::Result<T, String>;

/// Callbacks called by `Scale Mamba` during its operation.
pub trait ScaleMambaHandler: Send {
    fn close(&mut self) -> Result<()> {
        trace!("Scale Mamba calling close");
        Ok(())
    }

    fn open_channel(&self, channel: u32) -> Result<()> {
        trace!("Scale Mamba calling open channel: {}", channel);
        Ok(())
    }

    fn close_channel(&self, channel: u32) -> Result<()> {
        trace!("Scale Mamba calling close channel: {}", channel);
        Ok(())
    }

    fn private_input_gfp(&mut self, channel: u32) -> Result<Vec<u8>> {
        debug!("Scale Mamba calling private input gfp channel: {}", channel);
        Err(String::from("unimplemented"))
    }

    fn public_input_gfp(&self, channel: u32) -> Result<Vec<u8>> {
        debug!("Scale Mamba calling public input gfp channel: {}", channel);
        Err(String::from("unimplemented"))
    }

    fn public_input_int(&self, channel: u32) -> Result<i64> {
        debug!("Scale Mamba calling public input int channel: {}", channel);
        Err(String::from("unimplemented"))
    }

    fn private_output_gfp(&mut self, channel: u32, value: &[u8]) -> Result<()> {
        debug!(
            "Scale Mamba calling private output gfp channel: {}, value: {:?}",
            channel, value
        );
        Err(String::from("unimplemented"))
    }

    fn public_output_gfp(&self, channel: u32, value: &[u8]) -> Result<()> {
        debug!(
            "Scale Mamba calling public output gfp channel: {}, value: {:?}",
            channel, value
        );
        Err(String::from("unimplemented"))
    }

    fn public_output_int(&mut self, channel: u32, value: i64) -> Result<()> {
        trace!(
            "Scale Mamba calling public output int channel: {}, value: {} ",
            channel,
            value
        );
        Err(String::from("unimplemented"))
    }

    fn input_share(&self, channel: u32) -> Result<Vec<u8>> {
        debug!(
            "Scale Mamba calling public input share channel: {}",
            channel
        );
        Err(String::from("unimplemented"))
    }

    fn output_share(&self, channel: u32, value: &[u8]) -> Result<()> {
        debug!(
            "Scale Mamba calling output share channel: {}, value: {:?}",
            channel, value
        );
        Err(String::from("unimplemented"))
    }

    /// return the name of the next program to execute
    fn trigger(&mut self) -> Result<Vec<u8>>;

    /// The print output from the SCALE program.
    fn debug_output(&mut self, text: &[u8]);
}

fn catch_or_neg_one(f: impl FnOnce() -> Result<()>) -> c_int {
    std::panic::catch_unwind(std::panic::AssertUnwindSafe(f))
        .map_err(|_| error!("panic caught before going back to C"))
        .and_then(|val| val.map(|_| 0_i32).map_err(|e| error!("{}", e)))
        .unwrap_or(-1_i32)
}

#[no_mangle]
#[must_use]
pub extern "C" fn scale_io_close(handle: &mut ScaleMambaHandle) -> c_int {
    catch_or_neg_one(|| handle.handler.close())
}

#[no_mangle]
#[must_use]
pub extern "C" fn scale_io_open_channel(handle: &ScaleMambaHandle, channel: c_uint) -> c_int {
    catch_or_neg_one(|| handle.handler.open_channel(channel))
}

#[no_mangle]
#[must_use]
pub extern "C" fn scale_io_close_channel(handle: &ScaleMambaHandle, channel: c_uint) -> c_int {
    catch_or_neg_one(|| handle.handler.close_channel(channel))
}

#[no_mangle]
#[must_use]
pub extern "C" fn scale_io_private_input_gfp(
    result: &mut [u8; 512],
    handle: &mut ScaleMambaHandle,
    channel: c_uint,
) -> c_int {
    catch_or_neg_one(|| {
        let v = handle.handler.private_input_gfp(channel)?;
        result[..v.len()].copy_from_slice(&v);
        // null terminated
        result[v.len()] = 0;
        Ok(())
    })
}

#[no_mangle]
#[must_use]
pub extern "C" fn scale_io_public_input_gfp(
    result: &mut [u8; 512],
    handle: &ScaleMambaHandle,
    channel: c_uint,
) -> c_int {
    catch_or_neg_one(|| {
        let v = handle.handler.public_input_gfp(channel)?;
        result[..v.len()].copy_from_slice(&v);
        // null terminated
        result[v.len()] = 0;
        Ok(())
    })
}

#[no_mangle]
#[must_use]
pub extern "C" fn scale_io_public_input_int(
    result: &mut c_long,
    handle: &ScaleMambaHandle,
    channel: c_uint,
) -> c_int {
    catch_or_neg_one(|| {
        let v = handle.handler.public_input_int(channel)?;
        *result = v as c_long;
        Ok(())
    })
}

#[no_mangle]
#[must_use]
/// # Safety
/// Function should only be invoked from C FFI
/// None of the pointers given as an argument transfer ownership
pub unsafe extern "C" fn scale_io_private_output_gfp(
    handle: &mut ScaleMambaHandle,
    channel: c_uint,
    value: *const c_char,
) -> c_int {
    let value = CStr::from_ptr(value).to_bytes();
    catch_or_neg_one(|| handle.handler.private_output_gfp(channel, value))
}

#[no_mangle]
#[must_use]
/// # Safety
/// Function should only be invoked from C FFI
/// None of the pointers given as an argument transfer ownership
pub unsafe extern "C" fn scale_io_public_output_gfp(
    handle: &ScaleMambaHandle,
    channel: c_uint,
    value: *const c_char,
) -> c_int {
    let value = CStr::from_ptr(value).to_bytes();
    catch_or_neg_one(|| handle.handler.public_output_gfp(channel, value))
}

#[no_mangle]
#[must_use]
pub extern "C" fn scale_io_public_output_int(
    handle: &mut ScaleMambaHandle,
    channel: c_uint,
    value: c_long,
) -> c_int {
    #[allow(clippy::useless_conversion)] // Intellij bug - this conversion is NOT necessary
    let v: i64 = i64::from(value);

    catch_or_neg_one(|| handle.handler.public_output_int(channel, v))
}

#[no_mangle]
#[must_use]
pub extern "C" fn scale_io_input_share(
    result: &mut [u8; 512],
    handle: &ScaleMambaHandle,
    channel: c_uint,
) -> c_int {
    catch_or_neg_one(|| {
        let v = handle.handler.input_share(channel)?;
        result[..v.len()].copy_from_slice(&v);
        // null terminated
        result[v.len()] = 0;
        Ok(())
    })
}

#[no_mangle]
#[must_use]
/// # Safety
/// Function should only be invoked from C FFI
/// None of the pointers given as an argument transfer ownership
pub unsafe extern "C" fn scale_io_output_share(
    handle: &ScaleMambaHandle,
    channel: c_uint,
    value: *const c_char,
) -> c_int {
    let value = CStr::from_ptr(value).to_bytes();
    catch_or_neg_one(|| handle.handler.output_share(channel, value))
}

#[no_mangle]
#[must_use]
pub extern "C" fn scale_io_trigger(result: &mut [u8; 512], handle: &mut ScaleMambaHandle) -> c_int {
    catch_or_neg_one(|| {
        let v = handle.handler.trigger()?;
        result[..v.len()].copy_from_slice(&v);
        // null terminated
        result[v.len()] = 0;
        Ok(())
    })
}

#[no_mangle]
pub extern "C" fn scale_io_debug_output(text: *const c_char, handle: &mut ScaleMambaHandle) {
    catch_or_neg_one(|| unsafe {
        let value = CStr::from_ptr(text).to_bytes();
        handle.handler.debug_output(value);
        Ok(())
    });
}

/// The entry point for processing SCALE callbacks
#[repr(transparent)]
pub struct ScaleMambaHandle<'a> {
    pub handler: &'a mut dyn ScaleMambaHandler,
}
