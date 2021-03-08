#[derive(Debug)]
pub enum Error {
    Io(std::io::Error),
    Raw(String),
}

impl From<std::io::Error> for Error {
    fn from(err: std::io::Error) -> Self {
        Error::Io(err)
    }
}

impl From<String> for Error {
    fn from(s: String) -> Self {
        Error::Raw(s)
    }
}

impl<'a> From<&'a str> for Error {
    fn from(s: &'a str) -> Self {
        Error::Raw(s.to_string())
    }
}

impl<'a> From<anyhow::Error> for Error {
    fn from(e: anyhow::Error) -> Self {
        Error::Raw(e.to_string())
    }
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl std::error::Error for Error {}
