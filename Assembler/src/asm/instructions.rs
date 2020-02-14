use super::*;
use crate::lexer::{register, Operand, Register, RegisterKind};
use crate::span::Spanned;

#[derive(Clone, Debug)]
#[allow(clippy::large_enum_variant)]
pub enum IoInstruction<'a> {
    OpenChan {
        destination: Spanned<'a, register::Regint>,
    },
    CloseChan,
    PrivateInput {
        destination: Spanned<'a, register::Secret>,
        player: Spanned<'a, u32>,
    },
    PrivateOutput {
        value: Spanned<'a, register::Secret>,
        player: Spanned<'a, u32>,
    },
    InputShares {
        registers: Vec<Spanned<'a, register::Secret>>,
    },
    OutputShares {
        registers: Vec<Spanned<'a, register::Secret>>,
    },
    OutputClear {
        value: Spanned<'a, register::Clear>,
    },
    InputClear {
        destination: Spanned<'a, register::Clear>,
    },
    OutputInt {
        value: Spanned<'a, register::Regint>,
    },
    InputInt {
        destination: Spanned<'a, register::Regint>,
    },
}

check_type_size!(INSTRUCTION: Instruction<'static>, 160);

#[derive(Debug, Clone)]
#[allow(clippy::large_enum_variant)]
pub enum Instruction<'a> {
    /// `triple` instruction
    GenerateTriple {
        destinations: [Spanned<'a, register::Secret>; 3],
    },
    /// `bit` instruction
    GenerateBit {
        destination: Spanned<'a, register::Secret>,
    },
    /// `square` instruction
    GenerateSquare {
        destinations: [Spanned<'a, register::Secret>; 2],
    },
    Mul2Sint {
        destinations: [Spanned<'a, register::SecretRegint>; 2],
        values: [Spanned<'a, register::SecretRegint>; 2],
    },
    SintBit {
        destination: Spanned<'a, register::SecretRegint>,
        args: [Spanned<'a, Operand>; 3],
    },

    /// `ld*` and `mov*` instructions
    Assign {
        destination: Spanned<'a, Register>,
        value: Spanned<'a, Operand>,
    },
    /// `stm` instructions
    StoreToMemory {
        destination: Spanned<'a, Operand>,
        // FIXME: rename `RegisterBank` to `Bank` or sth
        memory_bank: MemoryBank,
        value: Spanned<'a, Register>,
    },
    /// `ldm` instructions
    LoadFromMemory {
        source: Spanned<'a, Operand>,
        memory_bank: MemoryBank,
        destination: Spanned<'a, Register>,
    },
    BinaryOperation {
        // FIXME: how much are these args dependent on the kind of register and operand?
        // can we eliminate the `instruction` field and just keep the register types?
        instruction: &'a str,
        op: BinaryOperation,
        a: Spanned<'a, Operand>,
        b: Spanned<'a, Operand>,
        destination: Spanned<'a, Register>,
    },
    UnaryOperation {
        op: UnaryOperation,
        a: Spanned<'a, Operand>,
        destination: Spanned<'a, Register>,
    },
    StartOpen {
        registers: Vec<Spanned<'a, register::Secret>>,
    },
    StopOpen {
        registers: Vec<Spanned<'a, register::Clear>>,
    },

    // Split the print instructions depending on their argument types
    PrintI {
        instruction: &'a str,
        arg: Spanned<'a, i32>,
    },

    PrintR {
        instruction: &'a str,
        arg: Spanned<'a, Register>,
    },

    PrintFL {
        args: [Spanned<'a, register::Clear>; 4],
    },

    PrintFix {
        args: [Spanned<'a, Operand>; 3],
    },

    Io {
        instr: IoInstruction<'a>,
        channel: Spanned<'a, u32>,
    },

    ///  Stack instructions
    PushR {
        value: Spanned<'a, Register>,
    },

    PopR {
        destination: Spanned<'a, Register>,
    },

    PeekR {
        destination: Spanned<'a, Register>,
        value: Spanned<'a, register::Regint>,
    },

    PokeR {
        value1: Spanned<'a, register::Regint>,
        value2: Spanned<'a, Register>,
    },

    GetSP {
        destination: Spanned<'a, register::Regint>,
        itype: RegisterKind,
    },

    /// `conv*` instructions
    Convert {
        value: Spanned<'a, Register>,
        destination: Spanned<'a, Register>,
        modp: Option<Spanned<'a, i32>>,
        unsignedint: bool,
    },

    /// Basically allows the compiler to work out a minimum prime which should be used,
    /// and then tell the runtime to crash if the user has violated this.
    /// - Needed for the current way we do floating/fix point arithmetic in MAMBA
    RequiredBitLength(Spanned<'a, i32>),

    JoinTape(Spanned<'a, i32>),

    RunTape {
        source: [Spanned<'a, i32>; 3],
    },

    Clock {
        /// Whether this is a start or a stop instruction
        stop: bool,
        id: Spanned<'a, i32>,
    },

    StArg {
        source: Spanned<'a, register::Regint>,
    },

    /// 'clear*'/`restart'/'crash' instructions
    VMControl(&'a str),

    LdTn {
        destination: Spanned<'a, register::Regint>,
    },

    LdArg {
        destination: Spanned<'a, register::Regint>,
    },

    OpenSint {
        destination: Spanned<'a, register::Regint>,
        source: Spanned<'a, register::SecretRegint>,
    },

    OpenSbit {
        destination: Spanned<'a, register::Regint>,
        source: Spanned<'a, register::SecretBit>,
    },

    DaBit {
        dest1: Spanned<'a, register::Secret>,
        dest2: Spanned<'a, register::SecretBit>,
    },

    // Invokes a garbled circuit
    GarbledCircuit(Spanned<'a, i32>),

    // Local Function call invocation
    LocalFunction(Spanned<'a, i32>),

    /// An instruction that doesn't exist in SCALE, will get optimized out
    /// during assembly.
    Nop,
}
