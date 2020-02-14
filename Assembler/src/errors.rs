use crate::compiler::substr_offset;
use crate::compiler::Error;
use crate::span::{Span, Spanned};
use crate::Compiler;

use annotate_snippets::snippet::{Annotation, AnnotationType, Slice, Snippet, SourceAnnotation};

pub struct InvalidVectorSize<'a> {
    pub n: Spanned<'a, i32>,
}

impl<'a> Error<'a> for InvalidVectorSize<'a> {
    fn spans(&self) -> &[Span<'a>] {
        std::slice::from_ref(&self.n.span)
    }
    fn print(self, cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some(format!(
                    "invalid vector length: expected value between 2 and 2^16, got `{}`",
                    self.n.elem
                )),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, "for vector length here", AnnotationType::Error),
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

pub struct ExpectedGot<'a, T, E> {
    pub got: Spanned<'a, T>,
    pub expected: E,
}

impl<'a, T: std::fmt::Display, E: std::fmt::Display> Error<'a> for ExpectedGot<'a, T, E> {
    fn spans(&self) -> &[Span<'a>] {
        std::slice::from_ref(&self.got.span)
    }
    fn print(self, cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some(format!(
                    "expected {}, got `{}`",
                    self.expected, self.got.elem
                )),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, "for operand here", AnnotationType::Error),
        }
    }
}

pub struct ExpectedOperand<'a> {
    pub span: Span<'a>,
}

impl<'a> Error<'a> for ExpectedOperand<'a> {
    fn spans(&self) -> &[Span<'a>] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some("expected an operand`".to_owned()),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, "in argument list here", AnnotationType::Error),
        }
    }
}

pub struct ArgNotFound<'a> {
    pub span: Span<'a>,
    pub i: usize,
}

impl<'a> Error<'a> for ArgNotFound<'a> {
    fn spans(&self) -> &[Span<'a>] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some(format!("expected at least {} arguments", self.i)),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, "", AnnotationType::Error),
        }
    }
}

pub struct JumpOutOfBounds<'a> {
    pub spans: Vec<Span<'a>>,
}

impl<'a> Error<'a> for JumpOutOfBounds<'a> {
    fn spans(&self) -> &[Span<'a>] {
        &self.spans
    }
    fn print(self, cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some("a jump jumped beyond the program's boundaries".to_owned()),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self
                .spans
                .into_iter()
                .map(|span| {
                    let (path, file) = cx.get_file(span).unwrap();
                    let (line_start, range, source) = line(span.snippet(), file);
                    Slice {
                        source,
                        line_start,
                        origin: Some(path.display().to_string()),
                        fold: false,
                        annotations: vec![SourceAnnotation {
                            label: "jumped from here".to_string(),
                            annotation_type: AnnotationType::Error,
                            range,
                        }],
                    }
                })
                .collect(),
        }
    }
}

pub struct UnknownInstruction<'a> {
    pub span: Span<'a>,
    pub instruction: &'a str,
}

impl<'a> Error<'a> for UnknownInstruction<'a> {
    fn spans(&self) -> &[Span<'a>] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some(format!(
                    "unknown or unimplemented instruction: {}",
                    self.instruction,
                )),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, "", AnnotationType::Error),
        }
    }
}

pub struct InvalidRegisterId<'a> {
    pub span: Span<'a>,
    pub err: std::num::ParseIntError,
}

impl<'a> Error<'a> for InvalidRegisterId<'a> {
    fn spans(&self) -> &[Span<'a>] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some("invalid register id".to_string()),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(cx, self.err.to_string(), AnnotationType::Error),
        }
    }
}

pub struct Io {
    pub err: std::io::Error,
}

impl<'a> Error<'a> for Io {
    fn spans(&self) -> &[Span<'a>] {
        std::slice::from_ref(&Span::DUMMY)
    }
    fn print(self, _cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some(self.err.to_string()),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: vec![],
        }
    }
}

pub struct UnimplementedInstruction<'a> {
    pub span: Span<'a>,
    pub name: &'a str,
}

impl<'a> Error<'a> for UnimplementedInstruction<'a> {
    fn spans(&self) -> &[Span<'a>] {
        std::slice::from_ref(&self.span)
    }
    fn print(self, cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some(format!("unimplemented instruction: {}", self.name)),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(
                cx,
                "cranelift backend does not yet support this instruction",
                AnnotationType::Error,
            ),
        }
    }
}

pub struct UninitializedRead<'a> {
    pub spans: Vec<Span<'a>>,
    // FIXME: make this a `Register`
    pub reg: u32,
}

impl<'a> Error<'a> for UninitializedRead<'a> {
    fn spans(&self) -> &[Span<'a>] {
        &self.spans
    }
    fn print(self, cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some("read from uninitialized register".to_string()),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: self.slices(
                cx,
                format!("register {} was never written to", self.reg),
                AnnotationType::Error,
            ),
        }
    }
}

pub struct DeadWrite<'a> {
    pub spans: Vec<Span<'a>>,
    // FIXME: make this a `Register`
    pub reg: u32,
}

impl<'a> Error<'a> for DeadWrite<'a> {
    fn spans(&self) -> &[Span<'a>] {
        &self.spans
    }
    fn fatal(&self) -> bool {
        false
    }
    fn print(self, cx: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some("Write to register is never read".to_string()),
                id: None,
                annotation_type: AnnotationType::Warning,
            }),
            footer: vec![],
            slices: self.slices(
                cx,
                format!(
                    "register {} written to but will never be read from again",
                    self.reg
                ),
                AnnotationType::Warning,
            ),
        }
    }
}

pub struct NotVectorizable {
    pub instr: &'static str,
    pub v: u32,
}

impl<'a> Error<'a> for NotVectorizable {
    fn spans(&self) -> &[Span<'a>] {
        std::slice::from_ref(&Span::DUMMY)
    }
    fn print(self, _: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some(format!(
                    "{:?} is not vectorizable but had a vector number of {}",
                    self.instr, self.v
                )),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: vec![],
        }
    }
}

pub struct SkippedErrors {
    pub error_count: usize,
}

impl<'a> Error<'a> for SkippedErrors {
    fn spans(&self) -> &[Span<'a>] {
        std::slice::from_ref(&Span::DUMMY)
    }
    fn print(self, _: &Compiler) -> Snippet {
        Snippet {
            title: Some(Annotation {
                label: Some(format!(
                    "{}/{} errors not shown",
                    self.error_count - 100,
                    self.error_count
                )),
                id: None,
                annotation_type: AnnotationType::Error,
            }),
            footer: vec![],
            slices: vec![],
        }
    }
}
