use crate::compiler::Compiler;
use crate::errors::ArgNotFound;

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub struct Span<'a> {
    pub snippet: &'a str,
    pub expansion: Expansion,
}

check_type_size!(SPAN: Span<'static>, 24);

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum Expansion {
    AlreadyErrored,
    Generated,
    None,
}

impl<'a> Span<'a> {
    pub fn generated(s: &'static str) -> Self {
        Span {
            snippet: s,
            expansion: Expansion::Generated,
        }
    }
    pub const DUMMY: Self = Span {
        snippet: "",
        expansion: Expansion::None,
    };
    pub fn with_error(self) -> Self {
        Span {
            expansion: Expansion::AlreadyErrored,
            ..self
        }
    }
    #[allow(clippy::reversed_empty_ranges)]
    // This empty range is on purpose, so we have the correct pointer address.
    pub fn start(self) -> Self {
        Span {
            snippet: &self.snippet[0..0],
            ..self
        }
    }
    pub fn end(self) -> Self {
        Span {
            snippet: &self.snippet[self.snippet.len()..],
            ..self
        }
    }
    pub fn is_empty(&self) -> bool {
        self.snippet.is_empty()
    }
    pub fn lines(file: &'a str) -> impl Iterator<Item = Span<'a>> + 'a {
        file.lines().map(move |snippet| Span {
            snippet,
            expansion: Expansion::None,
        })
    }
    pub fn split_at_first_char(self, c: char) -> (Self, Option<Self>) {
        match self.snippet().find(c) {
            None => (self, None),
            Some(pos) => {
                let first = Span {
                    snippet: &self.snippet[..pos],
                    ..self
                };
                let second = Span {
                    snippet: &self.snippet[pos + 1..],
                    ..self
                };
                (first, Some(second))
            }
        }
    }
    pub fn split(self, c: char) -> impl Iterator<Item = Span<'a>> + 'a {
        struct Iter<'a>(Option<Span<'a>>, char);
        impl<'a> Iterator for Iter<'a> {
            type Item = Span<'a>;
            fn next(&mut self) -> Option<Span<'a>> {
                let span = self.0.as_mut()?;
                let pos = match span.snippet().find(self.1) {
                    Some(pos) => pos,
                    None => return self.0.take(),
                };
                let ret = Span {
                    snippet: &span.snippet[..pos],
                    ..*span
                };
                span.snippet = &span.snippet[pos + 1..];
                Some(ret)
            }
        }
        Iter(Some(self), c)
    }
    pub fn trim(mut self) -> Self {
        let first = self.snippet().find(|c: char| !c.is_whitespace());
        if let Some(pos) = first {
            self.snippet = &self.snippet[pos..];
        } else {
            return self.start();
        }
        let last = self.snippet().rfind(|c: char| !c.is_whitespace());
        if let Some(pos) = last {
            self.snippet = &self.snippet[..=pos];
        } else {
            unreachable!()
        }
        self
    }
    pub fn snippet(&self) -> &'a str {
        &self.snippet
    }
    pub fn with<T>(self, elem: T) -> Spanned<'a, T> {
        Spanned { span: self, elem }
    }
}

#[derive(Debug, Eq, PartialEq, Copy, Clone)]
pub struct Spanned<'a, T> {
    pub span: Span<'a>,
    pub elem: T,
}

impl<'a, 'b, T: Copy + Default> Spanned<'a, &'b [Spanned<'a, T>]> {
    pub fn get(&self, i: usize) -> Option<Spanned<'a, T>> {
        self.elem.get(i).copied()
    }
    pub fn index_or_err(&self, cx: &'a Compiler, i: usize) -> Spanned<'a, T> {
        self.get(i).unwrap_or_else(|| {
            cx.report(ArgNotFound { span: self.span, i })
                .with_error()
                .with(Default::default())
        })
    }
    pub fn split_first_or_err(&self, cx: &Compiler) -> (Spanned<'a, T>, Self) {
        self.elem
            .split_first()
            .map(|(a, rest)| (*a, self.span.with(rest)))
            .unwrap_or_else(|| {
                let span = cx.report(ArgNotFound {
                    span: self.span,
                    i: 1,
                });
                (
                    span.with_error().with(Default::default()),
                    span.with_error().with(Default::default()),
                )
            })
    }
}

impl<'a, T> Spanned<'a, T> {
    pub fn map<U>(self, f: impl FnOnce(T) -> U) -> Spanned<'a, U> {
        Spanned {
            span: self.span,
            elem: f(self.elem),
        }
    }
    pub fn try_map<U, E>(
        self,
        f: impl FnOnce(T) -> Result<U, E>,
    ) -> Result<Spanned<'a, U>, Spanned<'a, E>> {
        match f(self.elem) {
            Ok(elem) => Ok(Spanned {
                elem,
                span: self.span,
            }),
            Err(elem) => Err(Spanned {
                elem,
                span: self.span,
            }),
        }
    }
}
