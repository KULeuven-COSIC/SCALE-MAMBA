// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::errors::SkippedErrors;
use crate::lexer::Lexical;
use annotate_snippets::{
    display_list::DisplayList,
    snippet::{AnnotationType, Slice, Snippet, SourceAnnotation},
};
use std::path::{Path, PathBuf};

use crate::asm::Body;
use crate::errors::ExpectedGot;
use crate::span::{Expansion, Span, Spanned};
use std::cell::{Cell, RefCell};
use std::{
    convert::TryInto,
    sync::{Arc, Mutex},
};

pub struct Compiler {
    destination: Option<RefCell<Box<dyn std::io::Write>>>,
    pub colors: bool,
    error_count: Cell<usize>,
    pub files: elsa::FrozenVec<String>,
    pub file_paths: elsa::FrozenVec<PathBuf>,
    pub binary_files: elsa::FrozenMap<PathBuf, Vec<u8>>,
    pub strings: elsa::FrozenVec<String>,
    pub show_warnings: bool,
    pub optimization_level: u8,
    pub verbose: bool,
    pub optimization_fuel: RefCell<Option<u64>>,
}

#[derive(Default, Clone)]
pub struct Helper(Arc<Mutex<Vec<u8>>>);
impl std::io::Write for Helper {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        self.0.lock().unwrap().write(buf)
    }
    fn flush(&mut self) -> std::io::Result<()> {
        Ok(())
    }
}
impl Helper {
    pub fn into_inner(self) -> String {
        String::from_utf8(Arc::try_unwrap(self.0).unwrap().into_inner().unwrap()).unwrap()
    }
}

impl Compiler {
    pub fn new(destination: Box<dyn std::io::Write>, colors: bool) -> Self {
        Compiler {
            destination: Some(RefCell::new(destination)),
            colors,
            files: Default::default(),
            file_paths: Default::default(),
            binary_files: Default::default(),
            strings: Default::default(),
            show_warnings: true,
            optimization_level: 3,
            error_count: Cell::new(0),
            verbose: false,
            optimization_fuel: std::env::var("OPTIMIZATION_FUEL")
                .ok()
                .map(|f| f.parse().unwrap())
                .into(),
        }
    }

    pub fn check_fuel_left(&self) -> bool {
        match &mut *self.optimization_fuel.borrow_mut() {
            None => true,
            Some(0) => false,
            Some(fuel) => {
                *fuel -= 1;
                true
            }
        }
    }

    pub fn parse_asm(&self, source: impl Into<PathBuf>, mut f: impl std::io::Read) -> Body<'_> {
        let mut text = String::new();
        if let Err(err) = f.read_to_string(&mut text) {
            self.report(crate::errors::Io { err });
        }
        self.parse_asm_buf(source, text)
    }

    pub fn lex<'a>(&'a self, file: u32) -> Vec<Lexical<'a>> {
        crate::lexer::parse(&self, file)
    }

    pub fn parse_asm_buf(&self, source: impl Into<PathBuf>, text: String) -> Body<'_> {
        let file = self.files.len();
        self.files.push(text);
        self.file_paths.push(source.into());
        let lexed = self.lex(file.try_into().unwrap());
        Body::parse(&self, &lexed)
    }

    pub fn parse_bytecode(
        &self,
        source: impl Into<PathBuf>,
        mut f: impl std::io::Read,
    ) -> Body<'_> {
        let mut text = Vec::new();
        if let Err(err) = f.read_to_end(&mut text) {
            self.report(crate::errors::Io { err });
        }
        let text = self.binary_files.insert(source.into(), text);
        let lexed = crate::binary::parse(&self, &text);
        Body::parse(&self, &lexed)
    }

    pub fn stderr() -> Self {
        Self::new(Box::new(std::io::stderr()), true)
    }
    /// Returns a span to be used for broken items, so they don't get more
    /// errors reported on them
    pub fn report(&self, err: impl Error) -> Span {
        let spans = err.spans();
        if let Some(&span) = spans
            .iter()
            .find(|s| s.expansion == Expansion::AlreadyErrored)
        {
            // still register an error count for subsequent errors
            if err.fatal() {
                self.error_count.set(self.error_count.get() + 1);
            }
            return span;
        }
        let span = spans[0];
        if err.fatal() {
            self.error_count.set(self.error_count.get() + 1);
        } else if !self.show_warnings {
            warn!(?err);
            return span;
        }
        error!(?err);
        if self.error_count.get() >= 100 {
            return span;
        }
        let dl = DisplayList::from(err.print(self));
        writeln!(self.destination.as_ref().unwrap().borrow_mut(), "{}", dl,).unwrap();
        if std::env::var("SCASM_CRASH_ON_ERROR").is_ok() {
            panic!()
        }
        span.with_error()
    }
    pub fn checked_from<
        T: std::convert::TryInto<U> + std::fmt::Display + std::fmt::Debug + Copy,
        U: Expected + Default,
    >(
        &self,
        got: Spanned<T>,
    ) -> Spanned<U> {
        got.try_map(T::try_into).unwrap_or_else(|_| {
            self.report(ExpectedGot {
                got,
                expected: U::EXPECTED,
            })
            .with(U::default())
        })
    }
    pub fn check_for_errors(&self) -> Result<(), ErrorReported> {
        trace!("{}", self.error_count.get());
        if self.error_count.get() > 0 {
            // this is a little whacky, but that's what we get for wanting a `done` cleanup function
            let error_count = self.error_count.get();
            if error_count > 100 {
                self.report(SkippedErrors { error_count });
            }
            self.error_count.set(0);
            Err(ErrorReported(()))
        } else {
            Ok(())
        }
    }

    pub fn get_file(&self, span: Span) -> Option<(&Path, &str)> {
        self.file_paths
            .iter()
            .zip(self.files.iter())
            .find(|(_, v)| substr_offset(span.snippet(&self), v).is_some())
    }
}

pub fn substr_offset(substr: &str, main_str: &str) -> Option<usize> {
    let sub_ptr = substr.as_bytes().as_ptr() as usize;
    let main_ptr = main_str.as_bytes().as_ptr() as usize;
    if main_ptr <= sub_ptr && (sub_ptr + substr.len()) <= (main_ptr + main_str.len()) {
        Some(sub_ptr - main_ptr)
    } else {
        None
    }
}

impl Drop for Compiler {
    fn drop(&mut self) {
        if !std::thread::panicking() && self.error_count.get() > 0 {
            panic!("do not drop `Compiler` types without checking for errors, use `.check_for_errors()` before doing so");
        }
    }
}

pub struct ErrorReported(());

impl std::fmt::Debug for ErrorReported {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt.write_str("Error already reported")
    }
}

pub trait Error: std::fmt::Debug + Sized {
    fn print(self, cx: &Compiler) -> Snippet<'_>;
    /// Whether compilation should not succeed
    fn fatal(&self) -> bool {
        true
    }
    fn spans(&self) -> &[Span];
    fn slices<'a>(
        &self,
        cx: &'a Compiler,
        label: &'a str,
        annotation_type: AnnotationType,
    ) -> Vec<Slice<'a>> {
        self.spans()
            .iter()
            .filter_map(|&span| cx.get_file(span).map(|(p, f)| (p, f, span)))
            .map(move |(path, file, span)| {
                let (line_start, range, source) = crate::errors::line(span.snippet(cx), file);
                Slice {
                    source: cx.strings.push_get(source),
                    line_start,
                    origin: Some(cx.strings.push_get(path.display().to_string())),
                    fold: false,
                    annotations: vec![SourceAnnotation {
                        label,
                        annotation_type,
                        range,
                    }],
                }
            })
            .collect()
    }
}

pub trait Expected {
    const EXPECTED: &'static str;
}
