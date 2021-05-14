// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::compiler::substr_offset;
use crate::compiler::Error;
use crate::span::{Span, Spanned};
use crate::Compiler;

use annotate_snippets::display_list::FormatOptions;
use annotate_snippets::snippet::{Annotation, AnnotationType, Slice, Snippet, SourceAnnotation};

#[derive(Debug)]
pub struct InvalidVectorSize {
    pub n: Spanned<i32>,
}

impl Error for InvalidVectorSize {
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&self.n.span)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some(cx.strings.push_get(format!(
                    "invalid vector length: expected value between 2 and 2^16, got `{}`",
                    self.n.elem
                ))),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, "for vector length here", AnnotationType::Error),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

/// Figures out the line number and string of the line that contains the substr
///
/// Note: this compares the string memory location, not the contents
pub fn line(substr: &str, s: &str) -> (usize, (usize, usize), String) {
    let (n, s, col) = s
        .lines()
        .enumerate()
        .find_map(|(n, line)| substr_offset(substr, line).map(|col| (n, line, col)))
        .unwrap();
    (n + 1, (col, col + substr.len()), s.to_owned())
}

#[derive(Debug)]
pub struct ExpectedGot<T, E> {
    pub got: Spanned<T>,
    pub expected: E,
}

impl<T: std::fmt::Display + std::fmt::Debug, E: std::fmt::Display + std::fmt::Debug> Error
    for ExpectedGot<T, E>
{
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&self.got.span)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some(cx.strings.push_get(format!(
                    "expected {}, got `{}`",
                    self.expected, self.got.elem
                ))),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, "for operand here", AnnotationType::Error),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct ExpectedOperand {
    pub span: Span,
}

impl Error for ExpectedOperand {
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some("expected an operand`"),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, "in argument list here", AnnotationType::Error),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct ArgNotFound {
    pub span: Span,
    pub i: usize,
}

impl Error for ArgNotFound {
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some(
                    cx.strings
                        .push_get(format!("expected at least {} arguments", self.i)),
                ),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, "", AnnotationType::Error),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct JumpOutOfBounds {
    pub spans: Vec<Span>,
}

impl Error for JumpOutOfBounds {
    fn spans(&self) -> &[Span] {
        &self.spans
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some("a jump jumped beyond the program's boundaries"),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self
                .spans
                .into_iter()
                .map(|span| {
                    let (path, file) = cx.get_file(span).unwrap();
                    let (line_start, range, source) = line(span.snippet(cx), file);
                    Slice {
                        source: cx.strings.push_get(source),
                        line_start,
                        origin: Some(cx.strings.push_get(path.display().to_string())),
                        fold: false,
                        annotations: vec![SourceAnnotation {
                            label: "jumped from here",
                            annotation_type: AnnotationType::Error,
                            range,
                        }],
                    }
                })
                .collect(),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct UnknownInstruction<'a> {
    pub span: Span,
    pub instruction: &'a str,
}

impl<'a> Error for UnknownInstruction<'a> {
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some(cx.strings.push_get(format!(
                    "unknown or unimplemented instruction: {}",
                    self.instruction,
                ))),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, "", AnnotationType::Error),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct InvalidRegisterId {
    pub span: Span,
    pub err: std::num::ParseIntError,
}

impl Error for InvalidRegisterId {
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some("invalid register id"),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(
                cx,
                cx.strings.push_get(self.err.to_string()),
                AnnotationType::Error,
            ),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct Io {
    pub err: std::io::Error,
}

impl Error for Io {
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&Span::DUMMY)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some(cx.strings.push_get(self.err.to_string())),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: vec![],
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct UnimplementedInstruction<'a> {
    pub span: Span,
    pub name: &'a str,
}

impl<'a> Error for UnimplementedInstruction<'a> {
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some(
                    cx.strings
                        .push_get(format!("unimplemented instruction: {}", self.name)),
                ),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(
                cx,
                "the instruction is not yet supported",
                AnnotationType::Error,
            ),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct TooManyArguments {
    pub span: Span,
}

impl Error for TooManyArguments {
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some(cx.strings.push_get("invalid number of arguments".to_string())),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(
                cx,
                "The number of arguments given did not match what the assembly instruction requires",
                AnnotationType::Error,
            ),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct UninitializedRead {
    pub spans: Vec<Span>,
    // FIXME: make this a `Register`
    pub reg: u32,
}

impl Error for UninitializedRead {
    fn spans(&self) -> &[Span] {
        &self.spans
    }
    fn fatal(&self) -> bool {
        // The scale engine allows reading from uninitialized registers,
        // which yields 0.
        false
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some("read from uninitialized register"),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(
                cx,
                cx.strings
                    .push_get(format!("register {} was never written to", self.reg)),
                AnnotationType::Error,
            ),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct DeadWrite {
    pub spans: Vec<Span>,
    // FIXME: make this a `Register`
    pub reg: u32,
}

impl Error for DeadWrite {
    fn spans(&self) -> &[Span] {
        &self.spans
    }
    fn fatal(&self) -> bool {
        false
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some("Write to register is never read"),
                id: None,
                annotation_type: AnnotationType::Warning,
            }),
            footer: vec![],
            slices: self.slices(
                cx,
                cx.strings.push_get(format!(
                    "register {} written to but will never be read from again",
                    self.reg
                )),
                AnnotationType::Warning,
            ),
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct NotVectorizable {
    pub instr: &'static str,
    pub v: u32,
}

impl Error for NotVectorizable {
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&Span::DUMMY)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some(cx.strings.push_get(format!(
                    "{:?} is not vectorizable but had a vector number of {}",
                    self.instr, self.v
                ))),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: vec![],
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}

#[derive(Debug)]
pub struct SkippedErrors {
    pub error_count: usize,
}

impl Error for SkippedErrors {
    fn spans(&self) -> &[Span] {
        std::slice::from_ref(&Span::DUMMY)
    }
    fn print(self, cx: &Compiler) -> Snippet<'_> {
        Snippet {
            title: Some(Annotation {
                label: Some(cx.strings.push_get(format!(
                    "{}/{} errors not shown",
                    self.error_count - 100,
                    self.error_count
                ))),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: vec![],
            opt: FormatOptions {
                color: cx.colors,
                ..Default::default()
            },
        }
    }
}
