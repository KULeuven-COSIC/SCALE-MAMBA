use crate::lexer::RegisterKind;
use documentation_parser::instructions;
use std::collections::HashMap;

lazy_static::lazy_static! {
    static ref NAME2INSTR: HashMap<String, &'static Instruction> = INSTRUCTIONS
        .iter()
        .flat_map(|group| group.instructions)
        .map(|i| (i.name.to_ascii_lowercase(), i))
        .collect();
}

pub fn name2instr(name: &str) -> Option<&'static Instruction> {
    NAME2INSTR.get(name).copied()
}

pub struct Instruction {
    pub name: &'static str,
    pub opcode: u32,
    pub args: &'static [Arg],
    pub vectorizable: bool,
    pub barrier: bool,
    pub thread_0_only: bool,
    pub mem_read: bool,
    pub mem_write: bool,
    pub terminator: bool,
    pub comment: &'static str,
    pub notes: &'static str,
}

pub struct Arg {
    pub name: &'static str,
    pub ty: ArgTy,
}

#[derive(Debug)]
pub enum ArgTy {
    /// Integer value
    Int {
        signed: bool,
        /// A static offset from the value in asm/bytecode to the actual value of the argument
        offset: i32,
    },
    Register(RegisterKind, RegisterMode),
    List {
        element_type: &'static ArgTy,
        /// Name of the argument that specifies the length of this array
        len_arg: &'static str,
    },
    Player,
    Channel,
    String,
}

impl std::fmt::Display for ArgTy {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            ArgTy::Int { signed, .. } => {
                if signed {
                    write!(fmt, "i")
                } else {
                    write!(fmt, "int")
                }
            }
            ArgTy::Player => write!(fmt, "p"),
            ArgTy::Channel => write!(fmt, "ch"),
            ArgTy::String => write!(fmt, "str"),
            ArgTy::List { element_type, .. } => write!(fmt, "*{}", element_type),
            ArgTy::Register(RegisterKind::Secret, RegisterMode::Write) => write!(fmt, "sw"),
            ArgTy::Register(RegisterKind::Regint, RegisterMode::Write) => write!(fmt, "rw"),
            ArgTy::Register(RegisterKind::SecretRegint, RegisterMode::Write) => write!(fmt, "srw"),
            ArgTy::Register(RegisterKind::Clear, RegisterMode::Write) => write!(fmt, "cw"),
            ArgTy::Register(RegisterKind::SecretBit, RegisterMode::Write) => write!(fmt, "sbw"),
            ArgTy::Register(RegisterKind::Secret, RegisterMode::Read) => write!(fmt, "s"),
            ArgTy::Register(RegisterKind::Regint, RegisterMode::Read) => write!(fmt, "r"),
            ArgTy::Register(RegisterKind::SecretRegint, RegisterMode::Read) => write!(fmt, "sr"),
            ArgTy::Register(RegisterKind::Clear, RegisterMode::Read) => write!(fmt, "c"),
            ArgTy::Register(RegisterKind::SecretBit, RegisterMode::Read) => write!(fmt, "sb"),
        }
    }
}

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum RegisterMode {
    Write,
    Read,
}

pub struct Group {
    pub name: &'static str,
    pub instructions: &'static [Instruction],
}

instructions! {
 "Load/store" {
  LDI & 0x1 & (dest: cw, value: i) & vectorizable & r##"LDI ci, n \newline
                                    Assigns (loads) clear register ci the value n. "## & ""
  LDSI & 0x2 & (dest: sw, value: i) & vectorizable & r##"LDSI si, n \newline
                                    Assigns secret register si a share of the value n.  "## & ""
  LDMC & 0x3 & (dest: cw, address: int) & vectorizable mem_read & r##"LDMC ci, n \newline
                                    Assigns clear register ci the value in memory C[n]. "## & ""
  LDMS & 0x4 & (dest: sw, address: int) & vectorizable mem_read & r##"LDMS si, n \newline
                                    Assigns secret register si the value in memory S[n]. "## & ""
  STMC & 0x5 & (value: c, address: int) & vectorizable mem_write & r##"STMC ci, n \newline
                                    Sets memory C[n] to be the value in clear register ci. "## & ""
  STMS & 0x6 & (value: s, address: int) & vectorizable mem_write & r##"STMS si n \newline
                                     Sets memory S[n] to be the value in secret register si. "## & ""
  LDMCI & 0x7 & (dest: cw, address: r) & vectorizable mem_read & r##"LDMCI ci, rj  \newline
                                    Assigns clear register ci the value in clear memory R[rj],
				    where rj is the j-th regint register.   "## & ""
  LDMSI & 0x8 &  (dest: sw, address: r) & vectorizable mem_read & r##"LDMSI si, rj \newline
                                    Assigns secret register si the value in secret memory S[rj],
				    where rj is the j-th regint register.   "## & ""
  STMCI & 0x9 & (value: c, address: r) & vectorizable mem_write & r##"STMCI ci, rj  \newline
                                    Sets clear memory C[rj] to be the value in clear register ci,
                                    where rj is the j-th regint register.   "## & ""
  STMSI & 0xA & (value: s, address: r) & vectorizable mem_write & r##"STMSI si, rj  \newline
                                    Sets secret memory S[rj] to be the value in secret register si,
				    where rj is the j-th regint register.   "## & ""
  MOVC & 0xB & (dest: cw, value: c) & vectorizable & r##"MOVC ci, cj \newline
                                    Assigns clear register ci the value in the clear register cj."## & ""
  MOVS & 0xC & (dest: sw, value: s) & vectorizable & r##"MOVS si, sj \newline
                                    Assigns secret register si the value in the secret register sj."## & ""
  MOVINT & 0xD & (dest: rw, value: r) & vectorizable & r##"MOVINT ri, rj \newline
                                    Assigns regint register ri the value in the regint register rj."## & ""
  LDMINT & 0xCA & (dest: rw, address: int) & vectorizable mem_read & r##"LDMINT ri, n \newline
                    Assigns regint register ri the value in memory R[n].  "## & ""
  STMINT & 0xCB & (value: r, address: int) & vectorizable mem_write & r##"STMINT ri, n \newline
                    Sets regint memory R[n] to be the value ri.  "## & ""
  LDMINTI & 0xCC & (dest: rw, address: r) & vectorizable mem_read & r##" LDMINTI ri, rj  \newline
                                       Assigns regint register ri the value in memory R[rj],
				       where rj is the j-th regint register.   "## & ""
  STMINTI & 0xCD & (src_addr: r, dest_addr: r) & vectorizable mem_write & r##" STMINTI ri, rj  \newline
                                      Sets regint memory R[rj] to be the value in regint register ri,
				      where rj is the j-th regint register.  "## & ""
},
  "Machine"  {
  LDTN & 0x10 & (dest: rw) & vectorizable & r##" LDTN ri \newline
                                 Assigns regint register ri the number of the current thread."## & ""
  LDARG & 0x11 & (dest: rw) & vectorizable mem_read & r##" LDARG ri \newline
                                 Assigns the argument passed to the current thread to the regint register ri.
                                 In MAMBA compiler this is also used to pass variables to functions."## & ""
  REQBL & 0x12 & (bitlength: int) & & r##"REQBL n \newline
                                  Defines the required bitlength for the modp arithmetic"## & ""
  STARG & 0x13 & (address: r) & vectorizable mem_write & r##"STARG ri \newline
                                 Assigns register ri to variable in the thread argument.
                                 In MAMBA compiler this is also used to pass variables to functions."## & ""
  RUN_TAPE & 0x19 & (thread: int, tape: int, argument: int) & barrier & r##"RUN\_TAPE i j n  \newline
                                           In thread i start tape n with argument j.  "## & ""
  JOIN_TAPE & 0x1A & (thread: int) & barrier & r##" JOIN\_TAPE i \newline
                                   Wait until tape in thread i has finished. "## & ""
  CRASH & 0x1B & () & terminator & r##"CRASH \newline
                                    Crash the runtime by calling CRASH on the IO class.  "## & ""
  RESTART & 0x1C & () & thread_0_only terminator & r##"RESTART \newline
                          Restart the runtime by reloading the schedule file.
                          Note, the schedule file may have changed since it has last been loaded.
                          See the main documentation for how this instruction is intended to be used.  "## & ""
  CLEAR_MEMORY & 0x1D & () & barrier& r##"CLEAR\_MEMORY \newline
         Clears the main memory. This can cause problems if executed in one thread
         and the memory is still being used in another. It is really for usage in
         thread zero, when all other threads are doing nothing. Say before a RESTART  "## & ""
  CLEAR_REGISTERS & 0x1E & () & barrier & r##"CLEAR\_REGISTERS \newline
         Like CLEAR\_MEMORY but this clears the registers of the current processor,
         i.e. within the current thread.  "## & ""
  },
  "Addition" {
  ADDC & 0x20  & (dest: cw, left: c, right: c) & vectorizable & r##"ADDC ci cj ck \newline
                                            Adds clear registers ci=cj+ck."## & ""
  ADDS & 0x21  & (dest: sw, left: s, right: s) & vectorizable & r##"ADDS si sj sk \newline
                                            Adds secret registers si=sj+sk."## & ""
  ADDM & 0x22  & (dest: sw, left: s, right: c) & vectorizable & r##"ADDM si sj ck \newline
                                            Adds clear register onto secret one si=sj+ck."## & ""
  ADDCI & 0x23  & (dest: cw, left: c, right: i) & vectorizable & r##"ADDCI ci cj n \newline
                                            Adds clear register to an immediate ci=cj+n."## & ""
  ADDSI & 0x24  & (dest: sw, left: s, right: i) & vectorizable & r##"ADDSI si sj n \newline
                                            Adds secret register to an immediate si=sj+n."## & ""
  SUBC & 0x25  & (dest: cw, left: c, right: c) & vectorizable & r##"SUBCC ci cj ck \newline
                                             Subtracts clear registers ci=cj-ck."## & ""
  SUBS & 0x26  & (dest: sw, left: s, right: s) & vectorizable & r##"SUBS si sj sk \newline
                                            Subtracts secret registers si=sj-sk."## & ""
  SUBML & 0x27  &  (dest: sw, left: s, right: c) & vectorizable & r##"SUBML si sj ck \newline
                                            Subtracts clear register from a secret one si=sj-ck."## & ""
  SUBMR & 0x28  &  (dest: sw, left: c, right: s) & vectorizable & r##"SUBML si cj sk \newline
                                            Subtracts secret register from a clear one si=cj-sk."## & ""
  SUBCI & 0x29  &  (dest: cw, left: c, right: i) & vectorizable & r##"SUBCI ci cj n \newline
                                            Subtract immediate value from a clear register ci=cj-n."## & ""
  SUBSI & 0x2A  &  (dest: sw, left: s, right: i) & vectorizable & r##"SUBSI si sj n \newline
                                            Subtract immediate value from a secret register si=sj-n."## & ""
  SUBCFI & 0x2B  & (dest: cw, left: c, right: i) & vectorizable & r##"SUBCFI ci cj n \newline
                                            Subtract clear register from an immediate  ci=n-cj."## & ""
  SUBSFI & 0x2C  &  (dest: sw, left: s, right: i) & vectorizable & r##"SUBSFI si sj n \newline
                                            Subtract secret register from an immediate  si=n-sj."## & ""
  },
  "Multiplication/division/other arithmetic" {
  MULC & 0x30  & (dest: cw, left: c, right: c) & vectorizable & r##"MULC ci cj ck \newline
                                             Multiplies clear registers ci=cj * ck."## & ""
  MULM & 0x31  & (dest: sw, left: s, right: c) & vectorizable & r##"MULM si sj ck \newline
                                             Multiplies secret and clear registers si=sj * ck."## & ""
  MULCI & 0x32  & (dest: cw, left: c, right: i) & vectorizable & r##"MULCI ci cj n \newline
                                             Multiplies clear register by an immediate ci=cj * n."## & ""
  MULSI & 0x33  & (dest: sw, left: s, right: i) & vectorizable & r##"MULSI si sj n \newline
                                             Multiplies secret register by an immediate si=sj * n."## & ""
  DIVC & 0x34  & (dest: cw, left: c, right: c) & vectorizable & r##"DIVC ci cj ck \newline
                                            Divides (mod p )  clear registers ci=cj / ck."## & ""
  DIVCI & 0x35  & (dest: cw, left: c, right: i) & vectorizable & r##"DIVCI ci cj n \newline
                                            Divides (mod p ) clear register by an immediate ci=cj / n. "## & ""
  MODC & 0x36  & (dest: cw, left: c, right: c) & vectorizable & r##"MODC ci cj ck \newline
                                            Clear division with remainder ci=(cj mod ck) (after lifting to the integers)."## & ""
  MODCI & 0x37  & (dest: cw, left: c, right: i) & vectorizable & r##"MODCI ci cj n \newline
                                            Clear division with remainder ci=(cj mod n) (after lifting to the integers) by an immediate."## & ""
  LEGENDREC & 0x38  & (dest: cw, value: c) & vectorizable & r##"LEGENDREC ci cj \newline
                         Legendre symbol computation on a clear register ci = (cj / p)."## & ""
  DIGESTC & 0x39  & (dest: cw, value: c) & vectorizable & r##"DIGESTC ci cj \newline
                                             Truncated hash computation on a clear register ci = H(cj)."## & ""
  },
  "IO" {
 OUTPUT_CLEAR & 0x40 & (value: c, channel: i) & vectorizable barrier thread_0_only & r##"OUTPUT\_CLEAR ci n \newline
                                            Public output of clear register ci to IO class on channel n. "## & ""
 INPUT_CLEAR & 0x41 & (dest: cw, channel: i) & vectorizable barrier thread_0_only & r##"INPUT\_CLEAR ci n \newline
					    Gets clear public input ci from the IO class on channel n.
                                            Public inputs need to be the same for all players running the protocol, otherwise a crash will occur. "## & ""
  OUTPUT_SHARES & 0x42 &  (n + 1: int, channel: ch, shares: [s; n]) & vectorizable barrier thread_0_only & r##"OUTPUT\_SHARES n+1 ch si1 ... sin \newline
           Write shares sij for j=1..n to the IO class channel ch.  "## & ""
  INPUT_SHARES & 0x43 & (n + 1: int, channel: i, shares: [sw; n]) & vectorizable barrier thread_0_only & r##"INPUT\_SHARES n+1 ch si1 ... sin \newline
          Read shares sij for j=1..n from the IO class channel ch.  "## & ""
  PRIVATE_INPUT & 0x44 & (dest: sw, player: p, channel: int) & vectorizable barrier thread_0_only & r##"PRIVATE\_INPUT si p m \newline
                                  Private input from player p on channel m assign result to secret si. "## & r##"c1"##
  PRIVATE_OUTPUT & 0x46  & (value: s, player: p, channel: int) & vectorizable barrier thread_0_only & r##"PRIVATE\_OUTPUT si p m \newline
                                  Private output to player p on channel m of secret si. "## & r##"c1"##
  OUTPUT_INT & 0x48 & (value: r, channel: i) & vectorizable barrier thread_0_only & r##" OUTPUT\_INT ri n \newline
                                            Public output of regint register ri to IO class on channel n. "## & ""
  INPUT_INT & 0x49 & (dest: rw, channel: i) & vectorizable barrier thread_0_only & r##"INPUT\_INT ri n \newline
					    Gets regint public input ri from the IO class on channel n.
                                            Public inputs need to be the same for all players running the protocol, otherwise a crash will occur. "## & ""
  OPEN_CHANNEL & 0x4A & (dest: rw, channel: i) & barrier thread_0_only & r##"OPEN\_CHANNEL ri n \newline
					  Opens channel number n for reading/writing on the IO class.
                                          Channels are assumed to be bi-directional, i.e. can read and write.
                                          This is provided as some IO classes may require this to be called
					  explicitly, the default one does not need this.
					  The return value ri {\bf can} be some error code which the IO class
            may want to return. "## & ""
  CLOSE_CHANNEL & 0x4B & (dest: i) & barrier thread_0_only & r##"CLOSE\_CHANNEL n \newline
					  Closes channel number n for reading/writing on the IO class.
                                          This is provided as some IO classes may require this to be called
            explicitly, the default one does not need this. "## & ""
  },
  "Open" {
  STARTOPEN & 0xA0 & (n: int, registers: [s; n]) & vectorizable & r##"STARTOPEN n, s1,...,sn \newline
                                         Start opening secret register si."## & "c0"
  STOPOPEN & 0xA1 & (n: int, registers: [cw; n]) & vectorizable & r##" STOPOPEN n, c1, ..., cn \newline
                                          Store previous opened value in clear register ci."## & "c0"
  OPENSINT & 0xA2 & (dest: rw, value: sr) & vectorizable & r##"OPENSINT ri sj \newline
                                           Open the secret regint in sj and assign it to ri.
                                           XXXX We will want to change this in future XXXXX"## & "c2"
  OPENSBIT & 0xA3 & (dest: rw, value: sb) & vectorizable & r##" OPENSBIT ri sbj \newline
                                           Open the sbit in sbj and assign it to ri.
                                           XXXX We will want to change this in future XXXXX"## & "c2"
  },
  "Data access" {
  TRIPLE & 0x50 & (dest_reg_i: sw, dest_reg_j: sw, dest_reg_k: sw) & vectorizable & r##"TRIPLE si sj sk \newline
                                        Load secret registers si, sj and sk with the next multiplication triple."## & ""
  BIT & 0x51  & (dest_reg: sw) & vectorizable & r##"BIT si \newline
                                 Load secret register si with the next secret bit."## & ""
  SQUARE & 0x52 & (dest_reg_i: sw, dest_reg_j: sw) & vectorizable & r##"SQUARE si sj \newline
                                        Load secret registers si and sj with the next square pair."## & ""
  DABIT & 0x53 & (dest_reg_i: sw, dest_reg_j: sbw) & vectorizable & r##" DABIT si sbj \newline
                                         Load secret, sbit registers si and sbj with the next secret dabit."## & ""
  },
  "sregint/sbit instructions" {
  LDMSINT & 0x60 & (dest: srw, address: int) & vectorizable mem_read & r##"LDMSINT si, n \newline
                                    Assigns secret regint register si the value in memory SR[n]. "## & ""
  LDMSINTI & 0x61 & (dest: srw, address: r) & vectorizable mem_read & r##"LDMSINTI si, rj \newline
                                    Assigns secret regint register si the value in secret memory SR[rj],
				    where rj is the j-th regint register.   "## & ""
  STMSINT & 0x62 & (value: sr, address: int) & vectorizable mem_write & r##"STMSINT si n \newline
                                     Sets memory SR[n] to be the value in sregint register si. "## & ""
  STMSINTI & 0x63 & (value: sr, address: r) & vectorizable mem_write & r##"STMSINT si rj \newline
                                     Sets memory SR[rj] to be the value in sregint register si. "## & ""
  MOVSINT & 0x64 & (dest: srw, value: sr) & vectorizable & r##"MOVSINT si sj \newline
                                   Assigns secure register si the value in the secure register sj."## & ""
  LDSINT & 0x65 & (dest: srw, value: i) & vectorizable & r##"LDSINT si n \newline
                                    Assigns sregint register si a share of the value n."## & ""
  LDSBIT & 0x7D & (dest: sbw, value: i) & vectorizable & r##"LDSBIT si n \newline
                                    Assigns sbit register si a share of the value n."## & ""
  ADDSINT & 0x66 & (dest: srw, left: sr, right: sr) & vectorizable & r##"ADDSINT si sj sk \newline
                                            Adds secret regint registers si=sj+sk."## & ""
  ADDSINTC & 0x67 & (dest: srw, left: sr, right: r) & vectorizable & r##"ADDSINTC si sj rk \newline
                                         Adds secret and clear registers si=sj+rk."## & ""
  SUBSINT & 0x68 & (dest: srw, left: sr, right: sr) & vectorizable & r##"SUBSINT si sj sk \newline
                                            Subtracts secret regint registers si=sj-sk."## & ""
  SUBSINTC & 0x69 & (dest: srw, left: sr, right: r) & vectorizable & r##"SUBSINTC si sj rk \newline
                                         Subtracts clear from secret register si=sj-rk."## & ""
  SUBCINTS & 0x6A  & (dest: srw, left: r, right: sr) & vectorizable & r##"SUBSINTC si sj rk \newline
                                         Subtracts secret from clear register si=rj-sk."## & ""
  MULSINT & 0x6B & (dest: srw, left: sr, right: sr) & vectorizable & r##"MULSINT si sj sk \newline
                                            Multiplies secret regint registers si=sj * sk."## & ""
  MULSINTC & 0x6C & (dest: srw, left: sr, right: r) & vectorizable & r##"MULSINTC si sj rk \newline
                                            Multiplies secret and clear regint registers si=sj * rk."## & ""
  DIVSINT & 0x6D & (dest: srw, left: sr, right: sr) & vectorizable & r##"DIVSINT si sj sk \newline
                                            Division of secret regint registers si=sj * sk."## & ""
  SHLSINT & 0x6E & (dest: srw, left: sr, right: int) & vectorizable & r##"SHLSINT i j k \newline
                                            Shift an sregint register left by k values"## & ""
  SHRSINT & 0x6F & (dest: srw, left: sr, right: int) & vectorizable & r##"SHRSINT i j k \newline
                                            Shift an sregint register right by k values"## & ""
  },
  "Bitwise logic" {
  ANDC & 0x70 & (dest: cw, left: c, right: c) & vectorizable & r##"ANDC ci cj ck \newline
                                   Lifts to integers, performs logical AND then converts back to modp, ci = cj and ck"## & ""
  XORC & 0x71 & (dest: cw, left: c, right: c) & vectorizable & r##"XORC ci cj ck \newline
                                   Lifts to integers, performs logical XOR then converts back to modp, ci = cj xor ck"## & ""
  ORC & 0x72 & (dest: cw, left: c, right: c) & vectorizable & r##"ORC ci cj ck \newline
                                   Lifts to integers, performs logical OR then converts back to modp, ci = cj or ck"## & ""
  ANDCI & 0x73 & (dest: cw, left: c, right: i) & vectorizable & r##"ANDCI ci cj n \newline
                                          Equivalent of ANDC with an immediate value ci = cj and n"## & ""
  XORCI & 0x74 & (dest: cw, left: c, right: i) & vectorizable & r##"XORCI ci cj n \newline
                                          Equivalent of XORC with an immediate value ci = cj xor n"## & ""
  ORCI & 0x75 & (dest: cw, left: c, right: i) & vectorizable & r##"ORCI ci cj n \newline
                                          Equivalent of ORC with an immediate value ci = cj or n"## & ""
  NOTC & 0x76 & (dest: cw, left: c, right: i) & vectorizable & r##"NOTC ci cj n \newline
                                       This takes the integer representation of the clear register cj,
                                       flips all the bits, adds $2^n$ and then converts back to clear modp register ci."## & ""
  NEG & 0x77 & (dest: srw, value: sr) & vectorizable & r##"NEG si sj \newline
                                       Negation of an sregint si=-sj"## & ""
  SAND & 0x78 & (dest: srw, left: sr, right: sb) & vectorizable & r##"SAND si sj sbk \newline
                                        ANDs the sregint with the sbit (in all bit positions) si= sj and sbk."## & ""
  XORSB & 0x79 & (dest: sbw, left: sb, right: sb) & vectorizable & r##"XORSB sbi sbj sbk \newline
                                      Secret XOR of sbit registers sbi = (sbj xor sbk)."## & ""
  ANDSB & 0x7A & (dest: sbw, left: sb, right: sb) & vectorizable & r##"ANDSB sbi sbj sbk \newline
                                      Secret AND of sbit registers sbi = (sbj and sbk)."## & ""
  ORSB & 0x7B & (dest: sbw, left: sb, right: sb) & vectorizable & r##"ORSB sbi sbj sbk \newline
                                      Secret OR of sbit registers sbi = (sbj or sbk)."## & ""
  NEGB & 0x7C & (dest: sbw, value: sb) & vectorizable & r##"NEGB sbi sbj \newline
                                       Secret NEG of sbit register sbi = 1-sbj."## & ""
  },
  "Bitwise shifts" {
  SHLC & 0x80 & (dest: cw, left: c, right: c) & vectorizable & r##"SHLC ci cj ck \newline
                                     Clear bitwise shift left of clear register ci = cj $\ll$ ck (after converting to integers)"## & ""
  SHRC & 0x81 & (dest: cw, left: c, right: c) & vectorizable & r##"SHRC ci cj ck \newline
                                     Clear bitwise shift right of clear register ci = cj $\gg$ ck (after converting to integers)"## & ""
  SHLCI & 0x82 & (dest: cw, left: c, right: i) & vectorizable & r##"SHLCI ci cj n \newline
                                     Clear bitwise shift left of clear register ci = cj $\ll$ n (after converting to integers)"## & ""
  SHRCI & 0x83 & (dest: cw, left: c, right: i) & vectorizable & r##"SHRCI ci cj n \newline
                                     Clear bitwise shift right of clear register ci = cj $\gg$ n (after converting to integers)"## & ""
  },
  "Branching and comparison" {
  JMP & 0x90 & (offset: int) & terminator & r##"JMP n \newline
                           Unconditional relative jump of n+1 instructions.  "## & ""
  JMPNZ & 0x91 & (condition: r, offset: int) & terminator & r##"JMPNZ ri n \newline
                          Jump of n+1 instructions if regint register ri is not equal to 0.
                          Example: \newline
            jmpnz c, n  : advance n+1 instructions if c is non-zero \newline
            jmpnz c, 0  : do nothing \newline
            jmpnz c, -1 : infinite loop if c is non-zero \newline          "## & ""
  JMPEQZ & 0x92 &  (condition: r, offset: int) & terminator & r##"JMPEQZ ri n \newline
                          Jump of n+1 instructions if regint register ri is equal to 0. "## & ""
  EQZINT & 0x93 & (dest: rw, value: r) & vectorizable & r##"EQZINT ri rj \newline
                                 Clear comparison to zero test of regint register ri = (rj == 0)."## & ""
  LTZINT & 0x94 & (dest: rw, value: r) & vectorizable & r##"LTZINT ri rj \newline
                                 Clear comparison of regint registers ri = (rj < 0)."## & ""
  LTINT & 0x95 & (dest: rw, left: r, right: r) & vectorizable & r##"LTINT ri rj rk \newline
                                 Clear comparison of regint registers ri = (rj < rk)."## & ""
  GTINT & 0x96& (dest: rw, left: r, right: r) & vectorizable & r##"GTINT ri rj rk \newline
                                 Clear comparison of regint registers ri = (rj > rk)."## & ""
  EQINT & 0x97 & (dest: rw, left: r, right: r) & vectorizable & r##"EQINT ri rj rk \newline
                                 Clear comparison of regint registers ri = (rj == rk)."## & ""
  CALL & 0x14 & (offset: int) & terminator & r##"CALL n \newline
                                Pushes the PC onto the stack, and does a relative jump of n+1 instructions "## & ""
  RETURN & 0x15& () & terminator & r##"RETURN \newline
                                Pops the top element off the stack, and assigns the PC to this value "## & ""
  },
  "Integers" {
  LDINT & 0x9A & (dest: rw, value: i) & vectorizable & r##"LDINT ri n \newline
                              Sets regint register ri to the immediate value n."## & ""
  ADDINT & 0x9B & (dest: rw, left: r, right: r) & vectorizable & r##"ADDINT ri rj rk \newline
                                     Addition of regint registers ri=rj + rk."## & ""
  SUBINT & 0x9C & (dest: rw, left: r, right: r) & vectorizable & r##"SUBINT ri rj rk \newline
                                     Subtraction of regint registers ri=rj - rk. "## & ""
  MULINT & 0x9D & (dest: rw, left: r, right: r) & vectorizable & r##"MULINT ri rj rk \newline
                                     Multiplication of regint registers ri=rj * rk. "## & ""
  DIVINT & 0x9E  & (dest: rw, left: r, right: r) & vectorizable & r##"DIVINT ri rj rk \newline
                                     Division of regint registers ri=rj / rk. "## & ""
  },
  "Conversion" {
  CONVINT & 0xC0 & (dest: cw, value: r) & vectorizable & r##"CONVINT ci rj \newline
                                 Convert from regint register rj to clear register ci."## & ""
  CONVMODP & 0xC1 & (dest: rw, value: c, bitlength: int) & vectorizable & r##"CONVMODP ri cj n \newline
                                 Convert from clear register cj to regint register ri with
                                 bitlength of cj equal to n"## & ""
  CONVSINTSREG & 0xC2 & (dest: srw, value: s) & vectorizable & r##"CONVSINTSREG si sj \newline
                                    Convert from secret  sj to sregint register si using DaBits"## & "c2"
  CONVREGSREG & 0xC3 & (dest: srw, value: r) & vectorizable & r##"CONVREGSREG si rj \newline
                                     Convert from regint register rj to secret register si."## & ""
  CONVSREGSINT & 0xC4 & (dest: sw, value: sr) & vectorizable & r##"CONVREGSINT si sj \newline
                                     Convert from secret (signed) regint register sj to secret register si using DaBits"## & "c2"
  CONVSUREGSINT & 0xC5 & (dest: sw, value: sr) & vectorizable & r##"CONVSUREGSINT si sj \newline
                                     Convert from secret (unsigned) regint register sj to secret register si using DaBits"## & "c2"
  },
  "Debug Printing" {
  PRINT_MEM & 0xB0 & (address: int) & vectorizable barrier thread_0_only& r##"PRINT\_MEM i \newline
                                      Print value in clear memory C[i] to debug IO channel.
                                      "## & ""
  PRINT_REG & 0xB2 & (value: c) & vectorizable barrier thread_0_only & r##"PRINT\_REG ci  \newline
                                 Print value of clear register ci to the debug IO channel.
                                 "## & ""
  PRINT_CHAR & 0xB3 & (value: i) & barrier thread_0_only & r##"PRINT\_CHAR i \newline
                               Prints the single character i to the debug IO channel.
                               "## & ""
  PRINT_CHAR4 & 0xB4 & (value: i) & barrier thread_0_only & r##"PRINT\_CHAR4 i \newline
                               Prints the 4 character string i to the debug IO channel.
                               "## & ""
  PRINT_CHAR_REGINT & 0xB5 & (value: r) & vectorizable barrier thread_0_only & r##" PRINT\_CHAR\_REGINT ri \newline
                               Prints the single character in regint ri to the debug IO channel.
                                "## & ""
  PRINT_CHAR4_REGINT & 0xB6 & (value: r) & vectorizable barrier thread_0_only & r##" PRINT\_CHAR4\_REGINT ri \newline
                                Prints regint ri as a four single character string to the debug IO channel.
                                "## & ""
  PRINT_FLOAT & 0xB7 &  (v: c, p: c, z: c, s: c) & vectorizable barrier thread_0_only & r##"PRINT\_FLOAT ci cj ck cl \newline
                                Prints the floating point number in clear registers (ci, cj, ck, cl)
                                assuming they map to the representation (v,p,z,s)
                               "## & ""
  PRINT_FIX & 0xB8 & (value: c, f: i, k: i) & vectorizable barrier thread_0_only & r##"PRINT\_FIX ci f k \newline
                                Prints the floating point number in clear register ci using
                                parameters f and k.
                                "## & ""
  PRINT_INT & 0xB9 & (value: r) & vectorizable barrier thread_0_only & r##"PRINT\_INT ri \newline
                               Prints the value of register ri to debug IO channel.
                               "## & ""
  PRINT_IEEE_FLOAT & 0xBA & (value: r) & vectorizable barrier thread_0_only & r##"PRINT\_IEEE\_FLOAT ri \newline
                               Prints the value of register ri to debug IO channel as a double
                               "## & ""
  },
  "Comparison of sregints" {
  EQZSINT  & 0xD0 & (dest: sbw, value: sr) & vectorizable & r##"EQZSINT sbi, sj \newline
                                   Equality test to zero of secret register sbi = (sj == 0)."## & ""
  LTZSINT  & 0xD1 & (dest: sbw, value: sr) & vectorizable & r##"EQZSINT sbi, sj \newline
                                   Less than test to zero of secret register sbi = (sj < 0)."## & ""
  },
  "Bitwise logical operations on sregints" {
  ANDSINT  & 0xD3 & (dest: srw, left: sr, right: sr) & vectorizable & r##"ANDSINT si sj sk \newline
                                       Bitwise ANDs secret registers si= sj and sk."## & ""
  ANDSINTC  & 0xD4  & (dest: srw, left: sr, right: r) & vectorizable & r##"ANDSINT si sj rk \newline
                                       Bitwise ANDs secret register with a clear si= sj and rk."## & ""
  ORSINT  & 0xD5  & (dest: srw, left: sr, right: sr) & vectorizable & r##"ORSINT si sj sk \newline
                                       Bitwise ORs secret registers si= sj or sk."## & ""
  ORSINTC  & 0xD6 & (dest: srw, left: sr, right: r) & vectorizable & r##"ORSINT si sj rk \newline
                                       Bitwise ORs secret register with a clear si= sj or rk."## & ""
  XORSINT  & 0xD7 & (dest: srw, left: sr, right: sr) & vectorizable & r##"XORSINT si sj sk \newline
                                       Bitwise XORs secret registers si= sj xor sk."## & ""
  XORSINTC  & 0xD8 & (dest: srw, left: sr, right: r) & vectorizable & r##"XORSINT si sj rk \newline
                                       Bitwise XORs secret register with a clear si= sj xor rk."## & ""
  INVSINT  & 0xD9 & (dest: srw, value: sr) & vectorizable & r##"INVSINT si sj \newline
                                        Bitwise inversion of the register si = INV sj."## & ""
  },
  r"64x64 $\longrightarrow$ 128 multiplier" {
  MUL2SINT  & 0xDA & (dest_most_sig: srw, dest_least_sig: srw, left: sr, right: sr) & vectorizable & r##" MUL2SINT si sj su sv \newline
                                             Full multiplication of secret registers
                                             (si || sj )=su *  sv. \newline
                                             Where si is the most significant word and sj is the least
                                             significant word of the output."## & ""
  },
  r"GC (and allied) commands" {
  GC  & 0xDB & (i: int) & mem_read mem_write & r##"GC i \newline
                            This calls the Garbled Circuit with index i."## & r##"c2"##
  BITSINT  & 0xDC & (dest: sbw, value: sr, bit: int) & vectorizable & r##"BITSINT sbi sj n \newline
                                                Assigns sbit register sbi the n-th bit of register sj."## & ""
  SINTBIT  & 0xDD  & (value: srw, dest: sr, bit_value: sb, bit_index: int) & vectorizable & r##"BITSINT si sj sbk n \newline
                                        Assigns sj to si, and then sets the n-th bit to be sbk"## & ""
  },
  "Others" {
  RAND & 0xE0 & (dest: rw, modulo: r) & vectorizable & r##"RAND ri rj \newline
                                    Writes to the regint register ri a random value reduced modulo rj
                                    The random value is the same for all players, so in particular it
                                    is not really random.
                                    More useful for randomization for Monte-Carlo algorithms"## & ""
  START_CLOCK & 0xE1 & (timer_id: i) & barrier & r##"START\_CLOCK n \newline
                                 Re-initializes the specified timer n "## & ""
  STOP_CLOCK & 0xE2 & (timer_id: i) & barrier & r##"STOP\_CLOCK n \newline
                                 Prints, to stdout (note {\bf not} the IO class) the time since the last initialization of timer n.
                                 "## & ""
},
"Local functions" {
  LF & 0xDE &  (i: int) & mem_read mem_write & r##"
                    LF\_CINT i \newline
                    This calls the Local Function with index i."## & ""
},
"Stack Operations" {
 PUSHINT & 0x100 & (value: r) & vectorizable mem_write & r##"PUSHINT ri \newline
                                 Push regint register ri onto the thread local stack"## & ""
 POPINT & 0x101 & (dest: rw) & vectorizable mem_read mem_write & r##"POPINT ri \newline
                                 Pop regint register ri from the thread local stack"## & ""
 PEEKINT & 0x102 & (dest: rw, value: r) & vectorizable mem_read & r##"PEEKINT ri, rj \newline
                                 Peeks at position pointed to by register rj from the thread-local regint stack and assigns to regint register ri."## & ""
 POKEINT & 0x103 & (value: r, value: r) & vectorizable mem_write & r##"POKEINT ri, rj \newline
                                 Replaces the data item pointed to by register ri on the thread-local regint local stack with the contents of register rj."## & ""
 GETSPINT & 0x104 & (dest: rw) & mem_read & r##"GETSPINT ri \newline
                                 Assigns the current stack pointer on the regint stack to register ri."## & ""
 PUSHSINT & 0x105 & (value: sr) & mem_write vectorizable & r##"PUSHSINT ri \newline
                                 Push secret regint register si onto the thread local stack"## & ""
 POPSINT & 0x106 & (dest: srw) & vectorizable mem_read mem_write & r##"POPSINT ri \newline
                                 Pop secret regint register si from the thread local stack"## & ""
 PEEKSINT & 0x107 & (dest: srw, value: r) & vectorizable mem_read & r##"PEEKSINT si, rj \newline
                                 Peeks at position pointed to by register rj from the thread-local secret regint stack and assigns to secret regint register si."## & ""
 POKESINT & 0x108 & (value: r, value: sr) & vectorizable mem_write & r##"POKESINT ri, sj \newline
                                 Replaces the data item pointed to by register ri on the thread-local secret regint local stack with the contents of register sj."## & ""
 GETSPSINT & 0x109 & (dest: rw) & mem_read & r##"GETSPSINT ri \newline
                                 Assigns the current stack pointer on the secret regint stack to register ri."## & ""
 PUSHSBIT & 0x10A & (value: sb) & vectorizable mem_write & r##"PUSHSBIT ri \newline
                                 Push secret bit register sbi onto the thread local stack"## & ""
 POPSBIT & 0x10B & (dest: sbw) & vectorizable mem_read mem_write & r##"POPSBIT ri \newline
                                 Pop secret bit register sbi from the thread local stack"## & ""
 PEEKSBIT & 0x10C & (dest: sbw, value: r) & vectorizable mem_read & r##"PEEKSBIT sbi, rj \newline
                                 Peeks at position pointed to by register rj from the thread-local secret bit stack and assigns to secret bit register sbi."## & ""
 POKESBIT & 0x10D & (value: r, value: sb) & vectorizable mem_write & r##"POKESBIT ri, sbj \newline
                                 Replaces the data item pointed to by register ri on the thread-local secret bit local stack with the contents of register sbj."## & ""
 GETSPSBIT & 0x10E & (dest: rw) & mem_read & r##"GETSPSBIT ri \newline
                                 Assigns the current stack pointer on the secret bit stack to register ri."## & ""
 PUSHC & 0x110 & (value: c) & vectorizable mem_write & r##"PUSHC ri \newline
                                 Push clear register ci onto the thread local stack"## & ""
 POPC & 0x111 & (dest: cw) & vectorizable mem_read mem_write & r##"POPC ri \newline
                                 Pop clear register ci from the thread local stack"## & ""
 PEEKC & 0x112 & (dest: cw, value: r) & vectorizable mem_read & r##"PEEKC ci, rj \newline
                                 Peeks at position pointed to by register rj from the thread-local clear stack and assigns to clear register ci."## & ""
 POKEC & 0x113 & (value: r, value: c) & vectorizable mem_write & r##"POKEC ri, cj \newline
                                 Replaces the data item pointed to by register ri on the thread-local clear local stack with the contents of register cj."## & ""
 GETSPC & 0x114 & (dest: rw) & mem_read & r##"GETSPC ri \newline
                                 Assigns the current stack pointer on the clear stack to register ri."## & ""
 PUSHS & 0x115 & (value: s) & vectorizable mem_write & r##"PUSHS ri \newline
                                 Push secret register si onto the thread local stack"## & ""
 POPS & 0x116 & (dest: sw) & vectorizable mem_read mem_write & r##"POPS ri \newline
                                 Pop secret register si from the thread local stack"## & ""
 PEEKS & 0x117 & (dest: sw, value: r) & vectorizable mem_read & r##"PEEKS si, rj \newline
                                 Peeks at position pointed to by register rj from the thread-local secret stack and assigns to secret register si."## & ""
 POKES & 0x118 & (value: r, value: s) & vectorizable mem_write & r##"POKES ri, sj \newline
                                 Replaces the data item pointed to by register ri on the thread-local secret local stack with the contents of register sj."## & ""
 GETSPS & 0x119 & (dest: rw) & mem_read & r##"GETSPS ri \newline
                                 Assigns the current stack pointer on the secret stack to register ri."## & ""
},

}
