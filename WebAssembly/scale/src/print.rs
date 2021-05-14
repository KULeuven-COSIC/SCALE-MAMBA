// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

pub trait Print {
    fn print(self);
}

impl Print for &'static str {
    #[inline(always)]
    fn print(self) {
        for &c in self.as_bytes() {
            unsafe { crate::__print_char_regint(c.into()) }
        }
    }
}

#[macro_export]
macro_rules! print {
    ($($e:expr),*) => {
        $($crate::Print::print($e);)*
    }
}

#[macro_export]
macro_rules! println {
    ($($e:expr),*) => {
        $($crate::Print::print($e);)*
        $crate::Print::print("\n");
    }
}

macro_rules! impl_print {
    ($target: ty) => {
        impl Print for $target {
            #[inline(always)]
            fn print(self) {
                unsafe { crate::__print_int(self as i64) }
            }
        }
    };
    ($($target: ty),*) => {
        $(
            impl Print for $target {
                #[inline(always)]
                fn print(self) {
                    unsafe { crate::__print_int(self as i64) }
                }
            }
        )*
    };
}

impl_print!(isize, usize, i128, u128, i64, u64, i32, u32, i16, u16, i8, u8, bool);
