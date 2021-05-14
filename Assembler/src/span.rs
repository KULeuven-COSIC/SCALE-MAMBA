// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::compiler::Compiler;
use crate::{errors::ArgNotFound, substr_offset};
use std::convert::{TryFrom, TryInto};

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub struct Span {
    file: u32,
    start: u32,
    end: u32,
    pub expansion: Expansion,
}

check_type_size!(SPAN: Span, 16);

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum Expansion {
    AlreadyErrored,
    Generated,
    None,
}

impl<'a> Span {
    pub fn new(file: u32, start: u32, end: u32, expansion: Expansion) -> Self {
        Self {
            file,
            start,
            end,
            expansion,
        }
    }
    pub fn generated() -> Self {
        Span {
            file: u32::max_value(),
            start: u32::max_value(),
            end: u32::max_value(),
            expansion: Expansion::Generated,
        }
    }
    pub const DUMMY: Self = Span {
        file: u32::max_value(),
        start: u32::max_value(),
        end: u32::max_value(),
        expansion: Expansion::None,
    };
    pub fn with_error(self) -> Self {
        Span {
            expansion: Expansion::AlreadyErrored,
            ..self
        }
    }
    pub fn start(self) -> Self {
        Span {
            end: self.start,
            ..self
        }
    }
    pub fn end(self) -> Self {
        Span {
            start: self.end,
            ..self
        }
    }
    pub fn is_empty(&self) -> bool {
        self.start == self.end
    }
    pub fn lines(cx: &'a Compiler, file: u32) -> impl Iterator<Item = Span> + 'a {
        let file_str = cx.files.get(file.try_into().unwrap()).unwrap();
        file_str.lines().map(move |snippet| {
            let start = substr_offset(snippet, file_str).unwrap();
            let start = u32::try_from(start).unwrap();
            Span {
                file,
                start,
                end: start + u32::try_from(snippet.len()).unwrap(),
                expansion: Expansion::None,
            }
        })
    }
    pub fn split_at_first_char(self, c: char, cx: &Compiler) -> (Self, Option<Self>) {
        match self.snippet(cx).find(c) {
            None => (self, None),
            Some(pos) => {
                let pos: u32 = pos.try_into().unwrap();
                let first = Span {
                    end: self.start + pos,
                    ..self
                };
                let second = Span {
                    start: self.start + pos + 1,
                    ..self
                };
                (first, Some(second))
            }
        }
    }
    pub fn split(self, c: char, cx: &'a Compiler) -> impl Iterator<Item = Span> + 'a {
        struct Iter<'a>(Option<Span>, char, &'a Compiler);
        impl<'a> Iterator for Iter<'a> {
            type Item = Span;
            fn next(&mut self) -> Option<Span> {
                let span = self.0.as_mut()?;
                let pos = match span.snippet(self.2).find(self.1) {
                    Some(pos) => pos,
                    None => return self.0.take(),
                };
                let pos = u32::try_from(pos).unwrap();
                let ret = Span {
                    end: span.start + pos,
                    ..*span
                };
                span.start += pos + 1;
                Some(ret)
            }
        }
        Iter(Some(self), c, cx)
    }
    pub fn trim(mut self, cx: &Compiler) -> Self {
        let first = self.snippet(cx).find(|c: char| !c.is_whitespace());
        if let Some(pos) = first {
            self.start += u32::try_from(pos).unwrap();
        } else {
            return self.start();
        }
        let last = self.snippet(cx).rfind(|c: char| !c.is_whitespace());
        if let Some(pos) = last {
            self.end = self.start + u32::try_from(pos).unwrap() + 1;
        } else {
            unreachable!()
        }
        self
    }
    pub fn snippet(self, cx: &Compiler) -> &str {
        cx.files
            .get(self.file as usize)
            .map_or("", |x| &x[self.start as usize..self.end as usize])
    }
    pub fn with<T>(self, elem: T) -> Spanned<T> {
        Spanned { span: self, elem }
    }
    /// Create a sub-snippet that references everything but the first character.
    /// Panics if the first character is larger than one byte.
    pub fn eat_one(mut self) -> Self {
        self.start += 1;
        self
    }
}

#[derive(Debug, Eq, PartialEq, Copy, Clone)]
pub struct Spanned<T> {
    pub span: Span,
    pub elem: T,
}

impl<T> From<T> for Spanned<T> {
    fn from(elem: T) -> Self {
        Span::DUMMY.with(elem)
    }
}

impl<'a, 'b, T: Copy + Default> Spanned<&'b [Spanned<T>]> {
    pub fn get(&self, i: usize) -> Option<Spanned<T>> {
        self.elem.get(i).copied()
    }
    pub fn index_or_err(&self, cx: &'a Compiler, i: usize) -> Spanned<T> {
        self.get(i).unwrap_or_else(|| {
            cx.report(ArgNotFound { span: self.span, i })
                .with_error()
                .with(Default::default())
        })
    }
    pub fn split_first_or_err(&self, cx: &Compiler) -> (Spanned<T>, Self) {
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

impl<'a, T> Spanned<T> {
    pub fn map<U>(self, f: impl FnOnce(T) -> U) -> Spanned<U> {
        Spanned {
            span: self.span,
            elem: f(self.elem),
        }
    }
    pub fn try_map<U, E>(
        self,
        f: impl FnOnce(T) -> Result<U, E>,
    ) -> Result<Spanned<U>, Spanned<E>> {
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
