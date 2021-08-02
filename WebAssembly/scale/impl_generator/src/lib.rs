// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

mod attrs;

extern crate proc_macro;
use attrs::AssignAttrs;
use documentation::{Arg, ArgTy, RegisterKind, RegisterMode};
use num_bigint::BigInt;
use num_integer::Integer;
use num_traits::identities::One;
use num_traits::sign::Signed;
use proc_macro::TokenStream;
use quote::quote;
use std::convert::TryFrom;
use syn::Error;

fn asm_ty(ty: &ArgTy) -> proc_macro2::TokenStream {
    match ty {
        ArgTy::Int { signed: true, .. } => quote!(i32),
        ArgTy::Int { signed: false, .. } => quote!(u32),
        ArgTy::Register(RegisterKind::Secret, ..) => quote!(crate::SecretModp),
        ArgTy::Register(RegisterKind::SecretBit, ..) => quote!(crate::RawSecretBit),
        ArgTy::Register(RegisterKind::Clear, ..) => quote!(crate::ClearModp),
        ArgTy::Register(RegisterKind::Regint, ..) => quote!(i64),
        ArgTy::Register(RegisterKind::Bit, ..) => quote!(bool),
        ArgTy::Register(RegisterKind::SecretRegint, ..) => quote!(crate::SecretI64),
        ArgTy::List { .. } => unimplemented!(),
    }
}

fn print_ty(ty: &ArgTy) -> proc_macro2::TokenStream {
    match ty {
        ArgTy::Int { signed: true, .. } => quote!(crate::ConstI32),
        ArgTy::Int { signed: false, .. } => quote!(crate::ConstU32),
        ArgTy::Register(RegisterKind::Secret, ..) => quote!(crate::SecretModp),
        ArgTy::Register(RegisterKind::SecretBit, ..) => quote!(crate::RawSecretBit),
        ArgTy::Register(RegisterKind::Clear, ..) => quote!(crate::ClearModp),
        ArgTy::Register(RegisterKind::Regint, ..) => quote!(i64),
        ArgTy::Register(RegisterKind::Bit, ..) => quote!(bool),
        ArgTy::Register(RegisterKind::SecretRegint, ..) => quote!(crate::SecretI64),
        ArgTy::List { .. } => unimplemented!(),
    }
}

fn ty_generics(
    ty: &ArgTy,
    generics: usize,
) -> (Option<proc_macro2::TokenStream>, Option<proc_macro2::Ident>) {
    let ty = match ty {
        ArgTy::Int { signed: true, .. } => Some(quote!(i32)),
        ArgTy::Int { signed: false, .. } => Some(quote!(u32)),
        ArgTy::Register(RegisterKind::Secret, ..) => None,
        ArgTy::Register(RegisterKind::SecretBit, ..) => None,
        ArgTy::Register(RegisterKind::Clear, ..) => None,
        ArgTy::Register(RegisterKind::Regint, ..) => None,
        ArgTy::Register(RegisterKind::Bit, ..) => None,
        ArgTy::Register(RegisterKind::SecretRegint, ..) => None,
        ArgTy::List { .. } => unimplemented!(),
    };
    match ty {
        None => (None, None),
        Some(ty) => {
            let ident = quote::format_ident!("{}", GENERICS[generics]);
            (Some(quote!(const #ident: #ty,)), Some(ident))
        }
    }
}

fn pop_ty_fn_name(ty: &ArgTy) -> &'static str {
    match ty {
        ArgTy::Int { signed: true, .. } => unimplemented!(),
        ArgTy::Int { signed: false, .. } => unimplemented!(),
        ArgTy::Register(RegisterKind::Secret, ..) => "secret_modp",
        ArgTy::Register(RegisterKind::SecretBit, ..) => "secret_bit",
        ArgTy::Register(RegisterKind::Clear, ..) => "clear_modp",
        ArgTy::Register(RegisterKind::Regint, ..) => "i64",
        ArgTy::Register(RegisterKind::Bit, ..) => "bool",
        ArgTy::Register(RegisterKind::SecretRegint, ..) => "secret_i64",
        ArgTy::List { .. } => unimplemented!(),
    }
}

fn check_ty_for_impl(ty: &ArgTy) -> bool {
    match ty {
        ArgTy::Int { .. }
        | ArgTy::Register(RegisterKind::Regint, ..)
        | ArgTy::Register(RegisterKind::Bit, ..) => false,
        ArgTy::Register(RegisterKind::Secret, ..)
        | ArgTy::Register(RegisterKind::SecretBit, ..)
        | ArgTy::Register(RegisterKind::Clear, ..)
        | ArgTy::Register(RegisterKind::SecretRegint, ..) => true,
        ArgTy::List { .. } => unimplemented!(),
    }
}

// Taken from
// https://github.com/dignifiedquire/num-bigint/blob/6f17ec8059c7b748ada4f015eddde993a8938033/src/algorithms/mod_inverse.rs#L11
fn mod_inverse(g: &BigInt, n: &BigInt) -> Option<BigInt> {
    let egcd = g.extended_gcd_lcm(n).0;
    let d = egcd.gcd;
    let x = egcd.x;

    if !d.is_one() {
        return None;
    }

    if x.is_negative() {
        Some(x + n)
    } else {
        Some(x)
    }
}

const GENERICS: &[&str] = &["A", "B", "C", "D"];

#[proc_macro]
pub fn impls(_: TokenStream) -> TokenStream {
    let groups = documentation::INSTRUCTIONS
        .iter()
        .enumerate()
        .map(|(i, group)| {
            let instructions = group.instructions.iter().filter_map(|instr| {
                let (args, ret) = split_args(instr.args)?;
                let ret_types = ret.iter().map(|r| print_ty(&r.ty));
                let arg_names: Vec<_> = args
                    .iter()
                    .map(|a| quote::format_ident!("__{}", a.name))
                    .collect();
                let arg_types: Vec<_> = args.iter().map(|a| print_ty(&a.ty)).collect();
                let name = instr.name.to_lowercase();
                let comments = instr.comment.lines().map(|line| {
                    let line = line.trim();
                    quote!(#[doc = #line])
                });
                let name_ident = quote::format_ident!("__{}", name);

                let impls = bin_op_impl(&name).and_then(|(trait_name, method_name)| {
                    // We use this flag to decide whether to generate commutative
                    // impls.
                    let commutative = match method_name {
                        "add" | "mul" | "bitxor" | "bitand" | "bitor" => {
                            match (&args[0].ty, &args[1].ty) {
                                // Only add a commutative impl if the types aren't the same
                                // in which case the commutative impl would just be the same
                                // as the original one and we'd get a conflict.
                                (ArgTy::Register(a, _), ArgTy::Register(b, _)) => a != b,
                                // With immediates we can also autogenerate the impls.
                                (ArgTy::Register(..), ArgTy::Int { .. }) => true,
                                _ => false,
                            }
                        }
                        _ => false,
                    };
                    let trait_name = quote::format_ident!("{}", trait_name);
                    assert_eq!(ret.len(), 1, "{}", name);
                    let ret_ty = print_ty(&ret[0].ty);
                    assert_eq!(args.len(), 2, "{}", name);
                    let self_ty = print_ty(&args[0].ty);
                    let (self_generics, self_params) = ty_generics(&args[0].ty, 0);
                    let other_ty = print_ty(&args[1].ty);
                    let (other_generics, other_params) = ty_generics(&args[1].ty, 1);
                    // ensure that we don't try to impl stuff for i64
                    if !(check_ty_for_impl(&args[0].ty) || check_ty_for_impl(&args[1].ty)) {
                        tracing::info!("skipping trait generation for {}", name);
                        return None;
                    }
                    let create_assign_op = match (&ret[0].ty, &args[0].ty) {
                        (ArgTy::Register(a, _), ArgTy::Register(b, _)) => a == b,
                        _ => false,
                    };
                    let assign_op = if create_assign_op {
                        let method_name = quote::format_ident!("{}_assign", method_name);
                        let trait_name = quote::format_ident!("{}Assign", trait_name);
                        Some(quote!(
                            impl<#self_generics #other_generics> core::ops::#trait_name<#other_ty<#other_params>> for #self_ty<#self_params> {
                                #[inline(always)]
                                fn #method_name(&mut self, other: #other_ty<#other_params>) {
                                    unsafe { *self = #name_ident(crate::AssemblyType::to_asm(*self), crate::AssemblyType::to_asm(other)); }
                                }
                            }
                        ))
                    } else {
                        None
                    };
                    let regular = create_binop(
                        false,
                        &trait_name,
                        &self_ty,
                        &self_generics,
                        &self_params,
                        &other_ty,
                        &other_generics,
                        &other_params,
                        method_name,
                        &ret_ty,
                        &name_ident,
                    );
                    let commutative = if commutative {
                        create_binop(
                            true,
                            &trait_name,
                            // Note the swapping of `other_ty` and `self_ty`
                            &other_ty,
                            &other_generics,
                            &other_params,
                            &self_ty,
                            &self_generics,
                            &self_params,
                            method_name,
                            &ret_ty,
                            &name_ident,
                        )
                    } else {
                        None
                    };
                    Some(quote!(
                        #regular
                        #commutative
                        #assign_op
                    ))
                });
                let store_impls = to_mem_ops(&name).and_then(|_| {
                    assert_eq!(ret.len(), 0, "{}", name);
                    assert_eq!(args.len(), 2, "{}", name);
                    let self_ty = print_ty(&args[0].ty);
                    let address = print_ty(&args[1].ty);
                    let (gen, par) = ty_generics(&args[1].ty, 0);
                    Some(quote!(
                        impl<#gen> crate::StoreInMem<#address<#par>> for #self_ty {
                            #[inline(always)]
                            unsafe fn store_in_mem(&self, addr: #address<#par>) {
                                unsafe { #name_ident(*self, crate::AssemblyType::to_asm(addr)) }
                            }
                        }
                    ))
                });
                let load_impls = from_mem_ops(&name).and_then(|_| {
                    assert_eq!(ret.len(), 1, "{}", name);
                    assert_eq!(args.len(), 1, "{}", name);
                    let self_ty = print_ty(&ret[0].ty);
                    let address = print_ty(&args[0].ty);
                    let (gen, par) = ty_generics(&args[0].ty, 0);
                    Some(quote!(
                        impl<#gen> crate::LoadFromMem<#address<#par>> for #self_ty {
                            #[inline(always)]
                            fn load_from_mem(addr: #address<#par>) -> #self_ty {
                                unsafe { #name_ident(crate::AssemblyType::to_asm(addr)) }
                            }
                        }
                    ))
                });
                let unop_impls = un_op_impl(&name).and_then(|(trait_name, method_name)| {
                    assert_eq!(ret.len(), 1, "{}", name);
                    assert_eq!(args.len(), 1, "{}", name);
                    let ret_ty = print_ty(&ret[0].ty);
                    let arg_ty = print_ty(&args[0].ty);
                    let method_name = quote::format_ident!("{}", method_name);
                    let trait_name = quote::format_ident!("{}", trait_name);
                    Some(quote!(
                        impl core::ops::#trait_name for #arg_ty {
                            type Output = #ret_ty;
                            #[inline(always)]
                            fn #method_name(self) -> #ret_ty {
                                unsafe { #name_ident(self) }
                            }
                        }
                    ))
                });
                let stack_impls = if name.starts_with("push") {
                    let self_ty = print_ty(&args[0].ty);
                    let root = name.trim_start_matches("push");
                    let push = quote::format_ident!("__push{}", root);
                    let pop = quote::format_ident!("__pop{}", root);
                    let peek = quote::format_ident!("__peek{}", root);
                    let poke = quote::format_ident!("__poke{}", root);
                    let getsp = quote::format_ident!("__getsp{}", root);
                    let rpeek = quote::format_ident!("__rpeek{}", root);
                    let rpoke = quote::format_ident!("__rpoke{}", root);
                    Some(quote!(
                        impl crate::Stack for #self_ty {
                            #[inline(always)]
                            unsafe fn push(val: &Self) {
                                unsafe { #push(*val) }
                            }
                            #[inline(always)]
                            unsafe fn pop() -> Self {
                                unsafe { #pop() }
                            }
                            #[inline(always)]
                            unsafe fn peek(addr: crate::StackAddress) -> Self {
                                unsafe { #peek(addr.0) }
                            }
                            #[inline(always)]
                            unsafe fn poke(addr: crate::StackAddress, val: &Self) {
                                unsafe { #poke(addr.0, *val) }
                            }
                            #[inline(always)]
                            unsafe fn peek_from_top(offset: i64) -> Self {
                                unsafe { #rpeek(offset) }
                            }
                            #[inline(always)]
                            unsafe fn poke_from_top(offset: i64, val: &Self) {
                                unsafe { #rpoke(offset, *val) }
                            }
                            #[inline(always)]
                            fn get_stack_pointer() -> crate::StackAddress {
                                unsafe { crate::StackAddress(#getsp()) }
                            }
                        }
                    ))
                } else {
                    None
                };
                let conv_impls = from_impls(&name).and_then(|()| {
                    assert_eq!(ret.len(), 1, "{}", name);
                    assert_eq!(args.len(), 1, "{}", name);
                    let self_ty = print_ty(&ret[0].ty);
                    let (gen, par) = ty_generics(&args[0].ty, 0);
                    let arg = print_ty(&args[0].ty);
                    Some(quote!(
                        impl<#gen> core::convert::From<#arg<#par>> for #self_ty {
                            #[inline(always)]
                            fn from(val: #arg<#par>) -> #self_ty {
                                unsafe { #name_ident(crate::AssemblyType::to_asm(val)) }
                            }
                        }
                    ))
                });
                let func = if ret_types.len() > 1 {
                    // Hacky system, we invoke a function that just pushes the values on the stack
                    // and then we call `pop` enough times to get said value
                    let pops = ret
                        .iter()
                        .map(|arg| quote::format_ident!("pop_{}", pop_ty_fn_name(&arg.ty)));
                    let hack_name = quote::format_ident!("push_multi_arg_{}", name);
                    quote!(
                        #(#comments)*
                        #[inline(always)]
                        #[cfg(not(feature = "emulate"))]
                        pub fn #name_ident(#(#arg_names: #arg_types),*) -> (#(#ret_types),*) {
                            extern "C" {
                                fn #hack_name(#(#arg_names: #arg_types),*);
                            }
                            unsafe {
                                #hack_name(#(#arg_names),*);
                                (#(crate::#pops()),*)
                            }
                        }
                    )
                } else {
                    let ret_types = ret.iter().map(|r| asm_ty(&r.ty));
                    let arg_types: Vec<_> = args.iter().map(|a| asm_ty(&a.ty)).collect();
                    let ret_type = match &*name {
                        "crash" | "restart" => quote!(crate::Never),
                        _ => quote!((#(#ret_types),*)),
                    };
                    quote!(
                        extern "C" {
                            #(#comments)*
                            pub fn #name_ident(#(#arg_names: #arg_types),*) -> #ret_type;
                        }
                    )
                };
                Some(quote!(
                    #func
                    #impls
                    #store_impls
                    #load_impls
                    #conv_impls
                    #stack_impls
                    #unop_impls
                ))
            });
            let group_name = group.name.trim();
            let group_id = quote::format_ident!("group_{}", i);
            quote! {
                #[doc = #group_name]
                mod #group_id {
                    #(#instructions)*
                }
                pub use #group_id::*;
            }
        });

    let sshare =
        std::fs::read_to_string(option_env!("SHARING_DATA").unwrap_or("./SharingData.txt"))
            .expect("SharingData.txt as specified in the SHARING_DATA env var does not exist");

    let p = BigInt::parse_bytes(sshare.lines().next().unwrap().as_bytes(), 10).unwrap();
    let param = (p.bits() as f64 / 8.0).ceil() * 8.0;
    let byte_length = ((param / 64.0).ceil() as usize) * 8;
    let r = BigInt::from(2).pow(8 * u32::try_from(byte_length).unwrap()) % &p;
    let r_inv = mod_inverse(&r, &p).unwrap_or_default();

    let p = bigint_to_bits(p);
    assert!(!p.is_empty());
    assert!(p.iter().any(|&b| b));
    let byte_length = u32::try_from(byte_length).unwrap();
    let r = bigint_to_bits(r);
    let r_inv = bigint_to_bits(r_inv);

    let expanded = quote! {
        #(#groups)*
        pub const P: &[bool] = &[#(#p),*];
        pub const BYTE_LENGTH: u32 = #byte_length;
        pub const R: &[bool] = &[#(#r),*];
        pub const R_INV: &[bool] = &[#(#r_inv),*];
    };
    TokenStream::from(expanded)
}

fn byte_to_bits(b: u8) -> impl Iterator<Item = bool> {
    (0..8).rev().map(move |i| (b >> i) & 1 == 1)
}

fn bigint_to_bits(int: BigInt) -> Vec<bool> {
    assert_eq!(int.sign(), num_bigint::Sign::Plus);
    assert_eq!(BigInt::from(515_u16).to_bytes_be().1, vec![2, 3]);
    assert_eq!(BigInt::from(14_u16).to_bytes_be().1, vec![14]);
    int.to_bytes_be()
        .1
        .into_iter()
        .flat_map(byte_to_bits)
        .collect()
}

fn create_binop(
    commutative: bool,
    trait_name: &proc_macro2::Ident,
    self_ty: &proc_macro2::TokenStream,
    self_generic: &Option<proc_macro2::TokenStream>,
    self_param: &Option<proc_macro2::Ident>,
    other_ty: &proc_macro2::TokenStream,
    other_generic: &Option<proc_macro2::TokenStream>,
    other_param: &Option<proc_macro2::Ident>,
    method_name: &str,
    ret_ty: &proc_macro2::TokenStream,
    name_ident: &proc_macro2::Ident,
) -> Option<proc_macro2::TokenStream> {
    let method_name = quote::format_ident!("{}", method_name);
    let op = if commutative {
        quote!(#name_ident(crate::AssemblyType::to_asm(other), crate::AssemblyType::to_asm(self)))
    } else {
        quote!(#name_ident(crate::AssemblyType::to_asm(self), crate::AssemblyType::to_asm(other)))
    };
    Some(quote!(
        impl<#self_generic #other_generic> core::ops::#trait_name<#other_ty<#other_param>> for #self_ty<#self_param> {
            type Output = #ret_ty;
            #[inline(always)]
            fn #method_name(self, other: #other_ty<#other_param>) -> Self::Output {
                unsafe { #op }
            }
        }
    ))
}

fn un_op_impl(name: &str) -> Option<(&str, &str)> {
    match name {
        "neg" => Some(("Neg", "neg")),
        _ => None,
    }
}

fn bin_op_impl(name: &str) -> Option<(&str, &str)> {
    if name.ends_with("sb") {
        return None;
    }
    match name {
        // These two have weird argument order
        "subsfi" | "subcfi" => return None,
        // This is not a binop, we're implementing that manually as it
        // is quite complex.
        "mul2sint" => return None,
        _ => {}
    }
    match name.get(..3)? {
        "add" => Some(("Add", "add")),
        "sub" => Some(("Sub", "sub")),
        "mul" => Some(("Mul", "mul")),
        "div" => Some(("Div", "div")),
        "mod" => Some(("Rem", "rem")),
        "xor" => Some(("BitXor", "bitxor")),
        "and" => Some(("BitAnd", "bitand")),
        "shl" => Some(("Shl", "shl")),
        "shr" => Some(("Shr", "shr")),
        _ => match name.get(..2)? {
            "or" => Some(("BitOr", "bitor")),
            _ => None,
        },
    }
}

fn to_mem_ops(name: &str) -> Option<&str> {
    match name.get(..3)? {
        "stm" => Some(&name[..3]),
        _ => None,
    }
}

fn from_mem_ops(name: &str) -> Option<&str> {
    match name.get(..3)? {
        "ldm" => Some(&name[..3]),
        _ => None,
    }
}

fn from_impls(name: &str) -> Option<()> {
    if name.contains("sbit") {
        return None;
    }
    match name {
        "ldsint" | "ldsbit" | "ldsi" | "ldi" | "ldint" => Some(()),
        // we have no concept of signedness
        "convsuregsint" => None,
        // This takes multiple args
        "convmodp" => None,
        _ if name.starts_with("conv") => Some(()),
        _ => None,
    }
}

fn split_args(all: &[Arg]) -> Option<(&[Arg], &[Arg])> {
    if all.iter().any(|arg| matches!(arg.ty, ArgTy::List { .. })) {
        return None;
    }
    for (i, arg) in all.iter().enumerate() {
        if let ArgTy::Register(_, RegisterMode::Write) = arg.ty {
            continue;
        }
        let (ret, arg) = all.split_at(i);
        return Some((arg, ret));
    }
    Some((&[], all))
}

#[proc_macro_attribute]
pub fn main(attrs: TokenStream, item: TokenStream) -> TokenStream {
    let fn_ = syn::parse_macro_input!(item as syn::ItemFn);

    let assign_attrs: AssignAttrs = match syn::parse(attrs) {
        Ok(attrs) => attrs,
        Err(_) => {
            return TokenStream::from(
                Error::new_spanned(fn_, "cannot parse attributes").to_compile_error(),
            );
        }
    };

    let left = assign_attrs.left;
    let right = assign_attrs.right;

    let res = quote! {
        scale::__main!{
            #(#left = #right;)*
        }

        #fn_
    };

    TokenStream::from(res)
}
