#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

scale::main! {
    I64_MEMORY = 0;
    SECRET_I64_MEMORY = 0;
    SECRET_MODP_MEMORY = 0;
    CLEAR_MODP_MEMORY = 0;
    KAPPA = 40;
}

#[inline(never)]
fn bar(x: i64) -> i64 {
    print!("asfdasdfasfasdfasdfasdfasdfasdfasdfasdfasdfadsfasdfasdfasdfasdfasdfadsfasdfasdf", x, "\n");
    x * 2
}


#[inline(never)]
fn main() {
    print!("foo");
    let y = bar(42);
    bar(43 + y).test();
}
