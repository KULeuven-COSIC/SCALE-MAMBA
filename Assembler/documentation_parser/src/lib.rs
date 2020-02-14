extern crate proc_macro;
use proc_macro::TokenStream;
use proc_macro2::Literal;
use quote::quote;
use syn::parse::{Parse, ParseStream, Result};
use syn::{parse_macro_input, Ident, Token};

struct Groups(proc_macro2::TokenStream);

struct Group(proc_macro2::TokenStream);

struct Instruction(proc_macro2::TokenStream);

struct Arg(proc_macro2::TokenStream);

fn parse_arg(input: ParseStream, offset: Literal) -> Result<proc_macro2::TokenStream> {
    let ty = match input.parse::<Ident>() {
        Ok(ty) => {
            let ty = ty.to_string();
            let ty = &*ty;
            match ty {
                "i" => quote!(ArgTy::Int { signed: true, offset: #offset }),
                "int" => quote!(ArgTy::Int { signed: false, offset: #offset }),
                "ch" => quote!(ArgTy::Channel),
                "p" => quote!(ArgTy::Player),
                "str" => quote!(ArgTy::String),
                _ => {
                    let (ty, mode) = if ty.ends_with('w') {
                        (&ty[..ty.len() - 1], quote!(RegisterMode::Write))
                    } else {
                        (ty, quote!(RegisterMode::Read))
                    };
                    let ty = match ty {
                        "s" => quote!(RegisterKind::Secret),
                        "c" => quote!(RegisterKind::Clear),
                        "r" => quote!(RegisterKind::Regint),
                        "sr" => quote!(RegisterKind::SecretRegint),
                        "sb" => quote!(RegisterKind::SecretBit),
                        _ => return Err(input.error(format!("unknown type {}", ty))),
                    };
                    quote!(ArgTy::Register(#ty, #mode))
                }
            }
        }
        Err(_) => {
            let content;
            syn::bracketed!(content in input);
            let elem = parse_arg(&content, Literal::u32_unsuffixed(0))?;
            content.parse::<Token![;]>()?;
            let len_arg: Ident = content.parse()?;
            let len_arg = len_arg.to_string();
            quote!( ArgTy::List {
                element_type: &#elem,
                len_arg: #len_arg,
            })
        }
    };
    Ok(ty)
}

impl Parse for Arg {
    fn parse(input: ParseStream) -> Result<Self> {
        let name = input.parse::<Ident>()?.to_string();
        let offset = if input.peek(Token![:]) {
            Literal::u32_unsuffixed(0)
        } else {
            input.parse::<Token![+]>()?;
            input.parse()?
        };
        input.parse::<Token![:]>()?;
        let ty = parse_arg(input, offset)?;
        Ok(Self(quote!(Arg {
            name: #name,
            ty: #ty,
        })))
    }
}

enum Flags {
    Vectorizable,
    Barrier,
    Thread0Only,
    MemRead,
    MemWrite,
    Terminator,
}

impl Parse for Flags {
    fn parse(input: ParseStream) -> Result<Self> {
        let v = input.parse::<Ident>()?.to_string();
        match &*v {
            "vectorizable" => Ok(Flags::Vectorizable),
            "barrier" => Ok(Flags::Barrier),
            "thread_0_only" => Ok(Flags::Thread0Only),
            "mem_read" => Ok(Flags::MemRead),
            "mem_write" => Ok(Flags::MemWrite),
            "terminator" => Ok(Flags::Terminator),
            _ => Err(input.error("expected `vectorizable` or `barrier`")),
        }
    }
}

impl Parse for Instruction {
    fn parse(input: ParseStream) -> Result<Self> {
        let name = input.parse::<Ident>()?.to_string();
        input.parse::<Token![&]>()?;
        let opcode = input.parse::<Literal>()?;
        input.parse::<Token![&]>()?;
        let stream;
        syn::parenthesized!(stream in input);
        let mut args = Vec::new();
        if !stream.is_empty() {
            loop {
                args.push(stream.parse::<Arg>()?.0);
                if stream.parse::<Token![,]>().is_err() {
                    break;
                }
            }
            assert!(stream.is_empty());
        }
        input.parse::<Token![&]>()?;
        let mut vectorizable = false;
        let mut barrier = false;
        let mut thread_0_only = false;
        let mut mem_write = false;
        let mut mem_read = false;
        let mut terminator = false;
        while !input.peek(Token![&]) {
            let set = |b: &mut bool| {
                assert!(!*b);
                *b = true;
            };
            match input.parse::<Flags>()? {
                Flags::Barrier => set(&mut barrier),
                Flags::Vectorizable => set(&mut vectorizable),
                Flags::Thread0Only => set(&mut thread_0_only),
                Flags::MemRead => set(&mut mem_read),
                Flags::MemWrite => set(&mut mem_write),
                Flags::Terminator => set(&mut terminator),
            }
        }
        if barrier && terminator {
            return Err(input
                .error("`terminator` flag implies `barrier`, no need to specify `barrier`, too"));
        }
        barrier |= terminator;
        if thread_0_only && !barrier {
            return Err(input.error("all `thread_0_only` instructions must be barriers"));
        }
        input.parse::<Token![&]>()?;
        let comment = input.parse::<Literal>()?;
        input.parse::<Token![&]>()?;
        let notes = input.parse::<Literal>()?;

        Ok(Self(quote!( Instruction {
            name: #name,
            opcode: #opcode,
            vectorizable: #vectorizable,
            barrier: #barrier,
            args: &[#(#args,)*],
            comment: #comment,
            notes: #notes,
            thread_0_only: #thread_0_only,
            mem_read: #mem_read,
            mem_write: #mem_write,
            terminator: #terminator,
        })))
    }
}

impl Parse for Group {
    fn parse(input: ParseStream) -> Result<Self> {
        let name = input.parse::<Literal>()?;
        let stream;
        syn::braced!(stream in input);
        let mut instructions = Vec::new();
        while !stream.is_empty() {
            instructions.push(stream.parse::<Instruction>()?.0);
        }
        input.parse::<Token![,]>()?;
        Ok(Self(quote!(Group {
            name: #name,
            instructions: &[#(#instructions,)*]
        })))
    }
}

impl Parse for Groups {
    fn parse(input: ParseStream) -> Result<Self> {
        let mut groups = Vec::new();
        while !input.is_empty() {
            groups.push(input.parse::<Group>()?.0);
        }
        Ok(Self(quote!( #(#groups,)* )))
    }
}

#[proc_macro]
pub fn instructions(input: TokenStream) -> TokenStream {
    let Groups(groups) = parse_macro_input!(input as Groups);
    let expanded = quote! {
        pub const INSTRUCTIONS: &[Group] = &[
            #groups
        ];
    };
    TokenStream::from(expanded)
}
