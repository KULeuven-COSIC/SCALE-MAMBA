// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use syn::{parse::Parse, punctuated::Punctuated, Error, Expr, ExprLit, Ident, Token};

#[derive(Default, Debug)]
pub struct AssignAttrs {
    pub(crate) left: Vec<Ident>,
    pub(crate) right: Vec<ExprLit>,
}
impl Parse for AssignAttrs {
    fn parse(input: syn::parse::ParseStream) -> syn::Result<Self> {
        let mut assign_attrs = AssignAttrs {
            left: vec![],
            right: vec![],
        };
        let attrs = Punctuated::<Expr, Token![,]>::parse_terminated(input)?;

        let assign_exprs = attrs.into_iter().filter_map(|attr| {
            if let Expr::Assign(assign_expr) = attr {
                match (*assign_expr.left, *assign_expr.right) {
                    (Expr::Path(path), Expr::Lit(lit)) => {
                        return Some((path, lit));
                    }
                    _ => return None,
                }
            }

            None
        });

        for (assign_expr, right_expr) in assign_exprs {
            let name = assign_expr.path.get_ident().ok_or_else(|| {
                Error::new_spanned(
                    assign_expr.clone(),
                    "main attribute option should have a name",
                )
            })?;

            assign_attrs.left.push(name.clone());
            assign_attrs.right.push(right_expr);
        }

        Ok(assign_attrs)
    }
}
