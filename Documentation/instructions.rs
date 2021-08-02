// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use documentation_parser::instructions;
use std::collections::HashMap;

#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash, Ord, PartialOrd)]
pub enum RegisterKind {
    Secret,
    Clear,
    Regint,
    SecretRegint,
    SecretBit,
    /// This is just the same thing as `Regint` from the scale perspective.
    /// It exists to make the wasm type system match up.
    Bit,
}

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

#[derive(Debug)]
pub struct Arg {
    pub name: &'static str,
    pub ty: ArgTy,
}

#[derive(Debug)]
pub enum ArgTy {
    /// Integer value
    Int {
        signed: bool,
        /// A static offset from the value in asm/byte-code to the actual value of the argument
        offset: i32,
    },
    Register(RegisterKind, RegisterMode),
    List {
        element_type: &'static ArgTy,
        /// Name of the argument that specifies the length of this array
        len_arg: &'static str,
    },
}

impl ArgTy {
    pub fn is_write(&self) -> bool {
        match self {
            Self::Register(_, RegisterMode::Write) => true,
            Self::List { .. } => unimplemented!(),
            _ => false,
        }
    }
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
            ArgTy::List { element_type, .. } => write!(fmt, "*{}", element_type),
            ArgTy::Register(RegisterKind::Secret, RegisterMode::Write) => write!(fmt, "sw"),
            ArgTy::Register(RegisterKind::Bit, RegisterMode::Write)
            | ArgTy::Register(RegisterKind::Regint, RegisterMode::Write) => write!(fmt, "rw"),
            ArgTy::Register(RegisterKind::SecretRegint, RegisterMode::Write) => write!(fmt, "srw"),
            ArgTy::Register(RegisterKind::Clear, RegisterMode::Write) => write!(fmt, "cw"),
            ArgTy::Register(RegisterKind::SecretBit, RegisterMode::Write) => write!(fmt, "sbw"),
            ArgTy::Register(RegisterKind::Secret, RegisterMode::Read) => write!(fmt, "s"),
            ArgTy::Register(RegisterKind::Bit, RegisterMode::Read)
            | ArgTy::Register(RegisterKind::Regint, RegisterMode::Read) => write!(fmt, "r"),
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
  MOVSB & 0xE & (dest: sbw, value: sb) & vectorizable & r##"MOVSB sbi, sbj \newline
                                    Assigns sbit register  sbi the value in the sbit register sbj."## & ""
  LDMINT & 0xCA & (dest: rw, address: int) & vectorizable mem_read & r##"LDMINT ri, n \newline
                    Assigns regint register ri the value in memory R[n].  "## & ""
  STMINT & 0xCB & (value: r, address: int) & vectorizable mem_write & r##"STMINT ri, n \newline
                    Sets regint memory R[n] to be the value ri.  "## & ""
  LDMINTI & 0xCC & (dest: rw, address: r) & vectorizable mem_read & r##" LDMINTI ri, rj  \newline
                                       Assigns regint register ri the value in memory R[rj],
				       where rj is the j-th regint register.   "## & ""
  STMINTI & 0xCD & (value: r, dest_addr: r) & vectorizable mem_write & r##" STMINTI ri, rj  \newline
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
  RUN_TAPE & 0x19 & (thread: int, tape: int, argument: int, argument: int) & barrier & r##"RUN\_TAPE i j k n  \newline
                                           In thread i start tape k with argument j.  \newline
                                           Starting with PC at position n."## & ""
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
  OUTPUT_CLEAR & 0x40 & (value: c, channel: r) & vectorizable barrier thread_0_only & r##"OUTPUT\_CLEAR ci rj \newline
                                            Public output of clear register ci to IO class on channel rj. "## & ""
  INPUT_CLEAR & 0x41 & (dest: cw, channel: r) & vectorizable barrier thread_0_only & r##"INPUT\_CLEAR ci rj \newline
					    Gets clear public input ci from the IO class on channel rj.
                                            Public inputs need to be the same for all players running the protocol, otherwise a crash will occur. "## & ""
  OUTPUT_SHARES & 0x42 &  (n + 1: int, channel: r, shares: [s; n]) & vectorizable barrier thread_0_only & r##"OUTPUT\_SHARES n+1 ri si1 ... sin \newline
           Write shares sij for j=1..n to the IO class channel ri.  "## & ""
  INPUT_SHARES & 0x43 & (n + 1: int, channel: r, shares: [sw; n]) & vectorizable barrier thread_0_only & r##"INPUT\_SHARES n+1 ri si1 ... sin \newline
          Read shares sij for j=1..n from the IO class channel ri.  "## & ""
  PRIVATE_INPUT & 0x44 & (dest: sw, player: r, channel: r) & vectorizable barrier thread_0_only & r##"PRIVATE\_INPUT si rj rk \newline
                                  Private input from player rj on channel rk assign result to secret si. "## & r##"c1"##
  PRIVATE_OUTPUT & 0x46  & (value: s, player: r, channel: r) & vectorizable barrier thread_0_only & r##"PRIVATE\_OUTPUT si rj rk \newline
                                  Private output to player rj on channel rk of secret si. "## & r##"c1"##
  OUTPUT_INT & 0x48 & (value: r, channel: r) & vectorizable barrier thread_0_only & r##" OUTPUT\_INT ri rj \newline
                                            Public output of regint register ri to IO class on channel rj. "## & ""
  INPUT_INT & 0x49 & (dest: rw, channel: r) & vectorizable barrier thread_0_only & r##"INPUT\_INT ri rj \newline
					    Gets regint public input ri from the IO class on channel rj.
                                            Public inputs need to be the same for all players running the protocol, otherwise a crash will occur. "## & ""
  OPEN_CHANNEL & 0x4A & (dest: rw, channel: r) & barrier thread_0_only & r##"OPEN\_CHANNEL ri rj \newline
					  Opens channel number rj for reading/writing on the IO class.
                                          Channels are assumed to be bi-directional, i.e. can read and write.
                                          This is provided as some IO classes may require this to be called
					  explicitly, the default one does not need this.
					  The return value ri {\bf can} be some error code which the IO class
            may want to return. "## & ""
  CLOSE_CHANNEL & 0x4B & (channel: r) & barrier thread_0_only & r##"CLOSE\_CHANNEL i \newline
					  Closes channel number ri for reading/writing on the IO class.
                                          This is provided as some IO classes may require this to be called
            explicitly, the default one does not need this. "## & ""
  MPRIVATE_INPUT & 0x4C & (value: r, value: r, player: r, channel: r) & mem_read mem_write vectorizable barrier thread_0_only & r##"MPRIVATE\_INPUT ri rj rk rl\newline
                                  Private input of rj items from player rk on channel rl assigning the result to sint memory [ri+t] for t=0...rj-1. "## & r##"c1"##
  MPRIVATE_OUTPUT & 0x4D  & (value: r, value: r, player: r, channel: r) & mem_read mem_write vectorizable barrier thread_0_only & r##"MPRIVATE\_OUTPUT ri rj rk rl\newline
                                  Private output of rj items from player rk on channel rl outputing the values in sint memory [ri+t] for t=0...rj-1. "## & r##"c1"##
  },
  "Open" {
  STARTOPEN & 0xA0 & (n: int, registers: [s; n]) & vectorizable & r##"STARTOPEN n, s1,...,sn \newline
                                         Start opening secret register si."## & "c0"
  STOPOPEN & 0xA1 & (n: int, registers: [cw; n]) & vectorizable & r##" STOPOPEN n, c1,..., cn \newline
                                          Store previous opened value in clear register ci."## & "c0"
  OPENSINT & 0xA2 & (dest: rw, value: sr) & vectorizable & r##"OPENSINT ri srj \newline
                                           Open the secret regint in srj and assign it to ri.
                                           XXXX We will want to change this in future XXXXX"## & "c2"
  OPENSBIT & 0xA3 & (dest: bw, value: sb) & vectorizable & r##" OPENSBIT ri sbj \newline
                                           Open the sbit in sbj and assign it to ri.
                                           XXXX We will want to change this in future XXXXX"## & "c2"
  },
  "Memory Management "{
  NEWC & 0xA4 & (dest: rw, value: r) & vectorizable mem_read & r##"NEWC ri, rj \newline
                                    Allocated rj locations of cint memory and returns the location in register ri.  "## & ""
  NEWS & 0xA5 & (dest: rw, value: r) & vectorizable mem_read & r##"NEWS ri, rj \newline
                                    Allocated rj locations of sint memory and returns the location in register ri.  "## & ""
  NEWINT & 0xA6 & (dest: rw, value: r) & vectorizable mem_read & r##"NEWINT ri, rj \newline
                                    Allocated rj locations of regint memory and returns the location in register ri.  "## & ""
  NEWSINT & 0xA7 & (dest: rw, value: r) & vectorizable mem_read & r##"NEWSINT ri, rj \newline
                                    Allocated rj locations of sregint memory and returns the location in register ri.  "## & ""
  DELETEC & 0xA8 & (value: r) & vectorizable mem_write & r##"DELETEC ri \newline
                                    Deletes the cint memory pointed to by regint register ri "## & ""
  DELETES & 0xA9 & (value: r) & vectorizable mem_write & r##"DELETES ri \newline
                                    Deletes the sint memory pointed to by regint register ri "## & ""
  DELETEINT & 0xAA & (value: r) & vectorizable mem_write & r##"DELETEINT ri \newline
                                    Deletes the regint memory pointed to by regint register ri "## & ""
  DELETESINT & 0xAB & (value: r) & vectorizable mem_write & r##"DELETESINT ri \newline
                                    Deletes the sregint memory pointed to by regint register ri "## & ""
  },
  "Data access" {
  TRIPLE & 0x50 & (n: int, registers: [sw; n]) & vectorizable & r##"TRIPLE n, s1,...,sn \newline
                                       Load sint registers s_{3i+1}, s_{3i+2} and s_{3i+3} with the next multiplication triple, for i=0,..,n/3-1. "## & ""
  BIT & 0x51 & (n: int, registers: [sw; n]) & vectorizable & r##"BIT n, s1,...,sn \newline
                                    Load sint register si with the next secret bit."## & ""
  SQUARE & 0x52 & (n: int, registers: [sw; n]) & vectorizable & r##"SQUARE n, s1,...,sn \newline
                                     Load sint registers s_{2i+1}, s_{2i+2} with the next square tuple, for i=0,..,n/2-1."## & ""
  DABIT & 0x53 & (dest_reg_i: sw, dest_reg_j: sbw) & vectorizable & r##" DABIT si sbj \newline
                                         Load secret, sbit registers si and sbj with the next secret dabit."## & ""
  },
  "Bitops on regints" {
  ANDINT  & 0x5A & (dest: rw, left: r, right: r) & vectorizable & r##"ANDINT ri rj rk \newline
                                       Bitwise ANDs registers ri= rj and rk."## & ""
  ORINT  & 0x5B  & (dest: rw, left: r, right: r) & vectorizable & r##"ORINT ri rj rk \newline
                                       Bitwise ORs registers ri= rj or rk."## & ""
  XORINT  & 0x5C & (dest: rw, left: r, right: r) & vectorizable & r##"XORINT ri rj rk \newline
                                       Bitwise XORs registers ri= rj xor rk."## & ""
  INVINT  & 0x5D & (dest: rw, value: r) & vectorizable & r##"INVINT ri rj \newline
                                        Bitwise inversion of the register ri = INV rj."## & ""
  SHLINT  & 0x5E & (dest: rw, left: r, right: r) & vectorizable & r##"SHLINT ri rj rk \newline
                                       Bitwise shift left register ri= rj << rk."## & ""
  SHRINT  & 0x5F & (dest: rw, left: r, right: r) & vectorizable & r##"SHRINT ri rj rk \newline
                                       Bitwise shift left register ri= rj >> rk."## & ""
  },
  "sregint/sbit instructions" {
  LDMSINT & 0x60 & (dest: srw, address: int) & vectorizable mem_read & r##"LDMSINT sri, n \newline
                                    Assrigns secret regint register sri the value in memory SR[n]. "## & ""
  LDMSINTI & 0x61 & (dest: srw, address: r) & vectorizable mem_read & r##"LDMSINTI sri, rj \newline
                                    Assrigns secret regint register sri the value in secret memory SR[rj],
				    where rj is the j-th regint register.   "## & ""
  STMSINT & 0x62 & (value: sr, address: int) & vectorizable mem_write & r##"STMSINT sri n \newline
                                     Sets memory SR[n] to be the value in sregint register sri. "## & ""
  STMSINTI & 0x63 & (value: sr, address: r) & vectorizable mem_write & r##"STMSINT sri rj \newline
                                     Sets memory SR[rj] to be the value in sregint register sri. "## & ""
  MOVSINT & 0x64 & (dest: srw, value: sr) & vectorizable & r##"MOVSINT sri srj \newline
                                   Assrigns secure register sri the value in the secure register srj."## & ""
  LDSINT & 0x65 & (dest: srw, value: i) & vectorizable & r##"LDSINT sri n \newline
                                    Assrigns sregint register sri a share of the value n."## & ""
  LDSBIT & 0x7D & (dest: sbw, value: i) & vectorizable & r##"LDSBIT sri n \newline
                                    Assrigns sbit register sri a share of the value n."## & ""
  ADDSINT & 0x66 & (dest: srw, left: sr, right: sr) & vectorizable & r##"ADDSINT sri srj srk \newline
                                            Adds secret regint registers sri=srj+srk."## & "c2"
  ADDSINTC & 0x67 & (dest: srw, left: sr, right: r) & vectorizable & r##"ADDSINTC sri srj rk \newline
                                         Adds secret and clear registers sri=srj+rk."## &  "c2"
  SUBSINT & 0x68 & (dest: srw, left: sr, right: sr) & vectorizable & r##"SUBSINT sri srj srk \newline
                                            Subtracts secret regint registers sri=srj-srk."## & "c2"
  SUBSINTC & 0x69 & (dest: srw, left: sr, right: r) & vectorizable & r##"SUBSINTC sri srj rk \newline
                                         Subtracts clear from secret register sri=srj-rk."## & "c2"
  SUBCINTS & 0x6A  & (dest: srw, left: r, right: sr) & vectorizable & r##"SUBSINTC sri srj rk \newline
                                         Subtracts secret from clear register sri=rj-srk."## & "c2"
  MULSINT & 0x6B & (dest: srw, left: sr, right: sr) & vectorizable & r##"MULSINT sri srj srk \newline
                                            Multiplies secret regint registers sri=srj * srk."## & "c2"
  MULSINTC & 0x6C & (dest: srw, left: sr, right: r) & vectorizable & r##"MULSINTC sri srj rk \newline
                                            Multiplies secret and clear regint registers sri=srj * rk."## & "c2"
  DIVSINT & 0x6D & (dest: srw, left: sr, right: sr) & vectorizable & r##"DIVSINT sri srj srk \newline
                                            Divisrion of secret regint registers sri=srj * srk."## & "c2"
  SHLSINT & 0x6E & (dest: srw, left: sr, right: int) & vectorizable & r##"SHLSINT sri srj k \newline
                                            Shift an sregint register left by k values"## & ""
  SHRSINT & 0x6F & (dest: srw, left: sr, right: int) & vectorizable & r##"SHRSINT sri srj k \newline
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
  NEG & 0x77 & (dest: srw, value: sr) & vectorizable & r##"NEG sri srj \newline
                                       Negation of an sregint sri=-srj"## & "c2"
  SAND & 0x78 & (dest: srw, left: sr, right: sb) & vectorizable & r##"SAND sri srj sbk \newline
                                        ANDs the sregint with the sbit (in all bit positions) sri= srj and sbk."## & "c2"
  XORSB & 0x79 & (dest: sbw, left: sb, right: sb) & vectorizable & r##"XORSB sbi sbj sbk \newline
                                      Secret XOR of sbit registers sbi = (sbj xor sbk)."## & ""
  ANDSB & 0x7A & (dest: sbw, left: sb, right: sb) & vectorizable & r##"ANDSB sbi sbj sbk \newline
                                      Secret AND of sbit registers sbi = (sbj and sbk)."## & "c2"
  ORSB & 0x7B & (dest: sbw, left: sb, right: sb) & vectorizable & r##"ORSB sbi sbj sbk \newline
                                      Secret OR of sbit registers sbi = (sbj or sbk)."## & "c2"
  NEGB & 0x7C & (dest: sbw, value: sb) & vectorizable & r##"NEGB sbi sbj \newline
                                       Secret NEG of sbit register sbi = 1-sbj."## & ""
  },
  "Bitwise operations mod p" {
  SHLC & 0x80 & (dest: cw, left: c, right: c) & vectorizable & r##"SHLC ci cj ck \newline
                                     Clear bitwise shift left of clear register ci = cj $\ll$ ck (after converting to integers)"## & ""
  SHRC & 0x81 & (dest: cw, left: c, right: c) & vectorizable & r##"SHRC ci cj ck \newline
                                     Clear bitwise shift right of clear register ci = cj $\gg$ ck (after converting to integers)"## & ""
  SHLCI & 0x82 & (dest: cw, left: c, right: i) & vectorizable & r##"SHLCI ci cj n \newline
                                     Clear bitwise shift left of clear register ci = cj $\ll$ n (after converting to integers)"## & ""
  SHRCI & 0x83 & (dest: cw, left: c, right: i) & vectorizable & r##"SHRCI ci cj n \newline
                                     Clear bitwise shift right of clear register ci = cj $\gg$ n (after converting to integers)"## & ""
  EQZC  & 0x84 & (dest: cw, left: c) & vectorizable & r##"EQZC ci cj \newline
                                     Sets cint register ci the value of (cj==0)"## & ""
  LTZC  & 0x85 & (dest: cw, left: c) & vectorizable & r##"LTZC ci cj \newline
                                     Sets cint register ci the value of (cj<0), i.e. whether the top bit of
cj is set"## & ""
  },
  "Branching and comparison" {
  JMP & 0x90 & (offset: int) & terminator & r##"JMP n \newline
                           Unconditional relative jump of n+1 instructions.  "## & ""
  JMPNE & 0x91 & (condition: r, value: int, offset: int) & terminator & r##"JMPNZ ri j n \newline
                          Jump of n+1 instructions if regint register ri is not equal to j.
                          Example: \newline
            jmpnz c, 0, n  : advance n+1 instructions if c is non-zero \newline
            jmpnz c, 0,  0  : do nothing \newline
            jmpnz c, 0,  -1 : infinite loop if c is non-zero \newline          "## & ""
  JMPEQ & 0x92 &  (condition: r, value: int, offset: int) & terminator & r##"JMPEQZ ri j n \newline
                          Jump of n+1 instructions if regint register ri is equal to j. "## & ""
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
  CALLR & 0x16 & (value: r) & terminator & r##"CALLR i \newline
                                Pushes the PC onto the stack, and then jumps to instruction at position ri"## & ""
  JMPR & 0x17 & (value: r) & terminator & r##"JMPR i \newline
                           Unconditional jump to instruction at address ri.  "## & ""
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
  MODINT & 0x9F  & (dest: rw, left: r, right: r) & vectorizable & r##"MODINT ri rj rk \newline
                                     Modular reduction of regint registers ri=rj \% rk. "## & ""
  },
  "Conversion" {
  CONVINT & 0xC0 & (dest: cw, value: r) & vectorizable & r##"CONVINT ci rj \newline
                                 Convert from regint register rj to clear register ci."## & ""
  CONVMODP & 0xC1 & (dest: rw, value: c, bitlength: int) & vectorizable & r##"CONVMODP ri cj n \newline
                                 Convert from clear register cj to regint register ri with
                                 bitlength of cj equal to n"## & ""
  CONVSINTSREG & 0xC2 & (dest: srw, value: s) & vectorizable & r##"CONVSINTSREG sri sj \newline
                                    Convert from secret  sj to sregint register sri using DaBits"## & "c2"
  CONVREGSREG & 0xC3 & (dest: srw, value: r) & vectorizable & r##"CONVREGSREG sri rj \newline
                                     Convert from regint register rj to secret register sri."## & ""
  CONVSREGSINT & 0xC4 & (dest: sw, value: sr) & vectorizable & r##"CONVREGSINT si srj \newline
                                     Convert from secret (signed) regint register srj to secret register si using DaBits"## & "c2"
  CONVSUREGSINT & 0xC5 & (dest: sw, value: sr) & vectorizable & r##"CONVSUREGSINT si srj \newline
                                     Convert from secret (unsigned) regint register srj to secret register si using DaBits"## & "c2"
  CONVSINTSBIT & 0xC6 & (dest: sbw, value: s) & vectorizable & r##"CONVSINTSBIT sbi srj \newline
                                     Convert from sint register srj to sbit register sbi using DaBits \newline
                                     Assumes srj holds only a bit. If not all security assumptions are invalid"## & "c2"
  CONVSBITSINT & 0xC7 & (dest: sw, value: sb) & vectorizable & r##"CONVSBITSINT sri sbj \newline
                                     Convert from sbit register sbj to sint register sri usinfg DaBits"## & "c2"
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
  PRINT_FLOAT & 0xB7 &  (v: c, p: c, z: c, s: c, e: c) & vectorizable barrier thread_0_only & r##"PRINT\_FLOAT ci cj ck cl cm \newline
                                Prints the floating point number in clear registers (ci, cj, ck, cl, cm)
                                assuming they map to the representation (v,p,z,s,err)
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
                                   Equality test to zero of secret register sbi = (sj == 0)."## & "c2"
  LTZSINT  & 0xD1 & (dest: sbw, value: sr) & vectorizable & r##"EQZSINT sbi, sj \newline
                                   Less than test to zero of secret register sbi = (sj < 0)."## & ""
  },
  "Bitwise logical operations on sregints" {
  ANDSINT  & 0xD3 & (dest: srw, left: sr, right: sr) & vectorizable & r##"ANDSINT sri srj srk \newline
                                       Bitwise ANDs secret registers sri= srj and srk."## & "c2"
  ANDSINTC  & 0xD4  & (dest: srw, left: sr, right: r) & vectorizable & r##"ANDSINT sri srj rk \newline
                                       Bitwise ANDs secret register with a clear sri= srj and rk."## & ""
  ORSINT  & 0xD5  & (dest: srw, left: sr, right: sr) & vectorizable & r##"ORSINT sri srj srk \newline
                                       Bitwise ORs secret registers sri= srj or srk."## & "c2"
  ORSINTC  & 0xD6 & (dest: srw, left: sr, right: r) & vectorizable & r##"ORSINT sri srj rk \newline
                                       Bitwise ORs secret register with a clear sri= srj or rk."## & ""
  XORSINT  & 0xD7 & (dest: srw, left: sr, right: sr) & vectorizable & r##"XORSINT sri srj srk \newline
                                       Bitwise XORs secret registers sri= srj xor srk."## & ""
  XORSINTC  & 0xD8 & (dest: srw, left: sr, right: r) & vectorizable & r##"XORSINT sri srj rk \newline
                                       Bitwise XORs secret register with a clear sri= srj xor rk."## & ""
  INVSINT  & 0xD9 & (dest: srw, value: sr) & vectorizable & r##"INVSINT sri srj \newline
                                        Bitwise inversion of the register sri = INV srj."## & ""
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
  SINTBIT  & 0xDD  & (value: srw, dest: sr, bit_value: sb, bit_index: int) & vectorizable & r##"SINTBIT si sj sbk n \newline
                                        Assigns sj to si, and then sets the n-th bit to be sbk"## & ""
  SETBIT   & 0xDF  & (value: srw, bit_value: sb, bit_index: int) & vectorizable & r##"SETBIT si sbk n \newline
                                        Assigns zero to si, and then sets the n-th bit to be sbk. \newline
                                        The assignment of zero, rather than take an existing register
                                        is to ensure we maintain SSA."## & ""
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
  RANDC & 0xE3 & (dest: cw) & vectorizable & r##"RANDC ci \newline
                            Writes to the cint register ci a random value mod p
                            The random value is the same for all players, so in particular it
                            is not really random.
                            More useful for randomization for Monte-Carlo algorithms"## & ""
  RANDINT & 0xE4 & (dest: rw) & vectorizable & r##"RANDINT ri \newline
                            Writes to the regint register ri a random value in the range $[0,..,2^{64}-1]$
                            The random value is the same for all players, so in particular it
                            is not really random.
                            More useful for randomization for Monte-Carlo algorithms"## & ""
  RANDSINT & 0xE5 & (dest: srw) & vectorizable & r##"RANDSINT sri \newline
                            Writes to the sregint register ri a (secret) random value in the 
                            range $[0,..,2^{64}-1]$ "## & ""
  RANDFLOAT & 0xE6 & (dest: rw) & vectorizable & r##"RANDFLOAT ri \newline
                            Writes to the regint register ri the IEEE representation of a floating point value in the range [0,1)
                            The random value is the same for all players, so in particular it
                            is not really random.
                            More useful for randomization for Monte-Carlo algorithms"## & ""
  RANDSBIT & 0xE7 & (dest: sbw) & vectorizable & r##"RANDSBIT sbi \newline
                            Writes to the sregint register sbi a (secret) random bit"## & ""
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
 POKEINT & 0x103 & (dest_ptr: r, value: r) & vectorizable mem_write & r##"POKEINT ri, rj \newline
                                 Replaces the data item pointed to by register ri on the thread-local regint local stack with the contents of register rj."## & ""
 GETSPINT & 0x104 & (dest: rw) & mem_read & r##"GETSPINT ri \newline
                                 Assigns the current stack pointer on the regint stack to register ri."## & ""
 PUSHSINT & 0x105 & (value: sr) & mem_write vectorizable & r##"PUSHSINT ri \newline
                                 Push secret regint register si onto the thread local stack"## & ""
 POPSINT & 0x106 & (dest: srw) & vectorizable mem_read mem_write & r##"POPSINT ri \newline
                                 Pop secret regint register si from the thread local stack"## & ""
 PEEKSINT & 0x107 & (dest: srw, value: r) & vectorizable mem_read & r##"PEEKSINT si, rj \newline
                                 Peeks at position pointed to by register rj from the thread-local secret regint stack and assigns to secret regint register si."## & ""
 POKESINT & 0x108 & (dest_ptr: r, value: sr) & vectorizable mem_write & r##"POKESINT ri, sj \newline
                                 Replaces the data item pointed to by register ri on the thread-local secret regint local stack with the contents of register sj."## & ""
 GETSPSINT & 0x109 & (dest: rw) & mem_read & r##"GETSPSINT ri \newline
                                 Assigns the current stack pointer on the secret regint stack to register ri."## & ""
 PUSHSBIT & 0x10A & (value: sb) & vectorizable mem_write & r##"PUSHSBIT ri \newline
                                 Push secret bit register sbi onto the thread local stack"## & ""
 POPSBIT & 0x10B & (dest: sbw) & vectorizable mem_read mem_write & r##"POPSBIT ri \newline
                                 Pop secret bit register sbi from the thread local stack"## & ""
 PEEKSBIT & 0x10C & (dest: sbw, value: r) & vectorizable mem_read & r##"PEEKSBIT sbi, rj \newline
                                 Peeks at position pointed to by register rj from the thread-local secret bit stack and assigns to secret bit register sbi."## & ""
 POKESBIT & 0x10D & (dest_ptr: r, value: sb) & vectorizable mem_write & r##"POKESBIT ri, sbj \newline
                                 Replaces the data item pointed to by register ri on the thread-local secret bit local stack with the contents of register sbj."## & ""
 GETSPSBIT & 0x10E & (dest: rw) & mem_read & r##"GETSPSBIT ri \newline
                                 Assigns the current stack pointer on the secret bit stack to register ri."## & ""
 PUSHC & 0x110 & (value: c) & vectorizable mem_write & r##"PUSHC ri \newline
                                 Push clear register ci onto the thread local stack"## & ""
 POPC & 0x111 & (dest: cw) & vectorizable mem_read mem_write & r##"POPC ri \newline
                                 Pop clear register ci from the thread local stack"## & ""
 PEEKC & 0x112 & (dest: cw, value: r) & vectorizable mem_read & r##"PEEKC ci, rj \newline
                                 Peeks at position pointed to by register rj from the thread-local clear stack and assigns to clear register ci."## & ""
 POKEC & 0x113 & (dest_ptr: r, value: c) & vectorizable mem_write & r##"POKEC ri, cj \newline
                                 Replaces the data item pointed to by register ri on the thread-local clear local stack with the contents of register cj."## & ""
 GETSPC & 0x114 & (dest: rw) & mem_read & r##"GETSPC ri \newline
                                 Assigns the current stack pointer on the clear stack to register ri."## & ""
 PUSHS & 0x115 & (value: s) & vectorizable mem_write & r##"PUSHS ri \newline
                                 Push secret register si onto the thread local stack"## & ""
 POPS & 0x116 & (dest: sw) & vectorizable mem_read mem_write & r##"POPS ri \newline
                                 Pop secret register si from the thread local stack"## & ""
 PEEKS & 0x117 & (dest: sw, value: r) & vectorizable mem_read & r##"PEEKS si, rj \newline
                                 Peeks at position pointed to by register rj from the thread-local secret stack and assigns to secret register si."## & ""
 POKES & 0x118 & (dest_ptr: r, value: s) & vectorizable mem_write & r##"POKES ri, sj \newline
                                 Replaces the data item pointed to by register ri on the thread-local secret local stack with the contents of register sj."## & ""
 GETSPS & 0x119 & (dest: rw) & mem_read & r##"GETSPS ri \newline
                                 Assigns the current stack pointer on the secret stack to register ri."## & ""
  RPEEKINT & 0x1F0 & (dest: rw, value: r) & vectorizable mem_read & r##"RPEEKINT ri, rj \newline
                                 Peeks at position pointed to by stack_pointer-rj from the thread-local regint stack and assigns to regint register ri."## & ""
  RPOKEINT & 0x1F1 & (dest_ptr: r, value: r) & vectorizable mem_write & r##"RPOKEINT ri, rj \newline
                                 Replaces the data item pointed to by stack_pointer-ri on the thread-local regint local stack with the contents of register rj."## & ""
  RPEEKSINT & 0x1F2 & (dest: srw, value: r) & vectorizable mem_read & r##"RPEEKSINT si, rj \newline
                                 Peeks at position pointed to by stack_pointer-rj from the thread-local secret regint stack and assigns to secret regint register si."## & ""
  RPOKESINT & 0x1F3 & (dest_ptr: r, value: sr) & vectorizable mem_write & r##"RPOKESINT ri, sj \newline
                                 Replaces the data item pointed to by stack_pointer-ri on the thread-local secret regint local stack with the contents of register sj."## & ""
  RPEEKSBIT & 0x1F4 & (dest: sbw, value: r) & vectorizable mem_read & r##"RPEEKSBIT sbi, rj \newline
                                 Peeks at position pointed to by stack_pointer-rj from the thread-local secret bit stack and assigns to secret bit register sbi."## & ""
  RPOKESBIT & 0x1F5 & (dest_ptr: r, value: sb) & vectorizable mem_write & r##"RPOKESBIT ri, sbj \newline
                                 Replaces the data item pointed to by stack_pointer-ri on the thread-local secret bit local stack with the contents of register sbj."## & ""
  RPEEKC & 0x1F6 & (dest: cw, value: r) & vectorizable mem_read & r##"RPEEKC ci, rj \newline
                                 Peeks at position pointed to by stack_pointer-rj from the thread-local clear stack and assigns to clear register ci."## & ""
  RPOKEC & 0x1F7 & (dest_ptr: r, value: c) & vectorizable mem_write & r##"RPOKEC ri, cj \newline
                                 Replaces the data item pointed to by ri on the thread-local clear local stack with the contents of register cj."## & ""
  RPEEKS & 0x1F8 & (dest: sw, value: r) & vectorizable mem_read & r##"RPEEKS si, rj \newline
                                 Peeks at position pointed to by stack_pointer-rj from the thread-local secret stack and assigns to secret register si."## & ""
  RPOKES & 0x1F9 & (dest_ptr: r, value: s) & vectorizable mem_write & r##"RPOKES ri, sj \newline
                                 Replaces the data item pointed to by stack_pointer-ri on the thread-local secret local stack with the contents of register sj."## & ""
},
"Memory Based Array Instructions" {
  MADDC & 0x120 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MADDC ri rj rk rn \newline
                                 C[ri+t] = C[rj+t] + C[rk+t] for t=0...(rn-1).  "## & ""
  MADDS & 0x121 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MADDS ri rj rk rn \newline
                                 S[ri+t] = S[rj+t] + S[rk+t] for t=0...(rn-1).  "## & ""
  MADDM & 0x122 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MADDM ri rj rk rn \newline
                                 S[ri+t] = S[rj+t] + C[rk+t] for t=0...(rn-1).  "## & ""
  MSUBC & 0x125 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MSUBC ri rj rk rn \newline
                                 C[ri+t] = C[rj+t] - C[rk+t] for t=0...(rn-1).  "## & ""
  MSUBS & 0x126 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MSUBS ri rj rk rn \newline
                                 S[ri+t] = S[rj+t] - S[rk+t] for t=0...(rn-1).  "## & ""
  MSUBML & 0x127 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MSUBML ri rj rk rn \newline
                                 S[ri+t] = S[rj+t] - C[rk+t] for t=0...(rn-1).  "## & ""
  MSUBMR & 0x128 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MSUBMR ri rj rk rn \newline
                                 S[ri+t] = C[rj+t] - S[rk+t] for t=0...(rn-1).  "## & ""
  MMULC & 0x130 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MMULC ri rj rk rn \newline
                                 C[ri+t] = C[rj+t] * C[rk+t] for t=0...(rn-1).  "## & ""
  MMULM & 0x131 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MMULM ri rj rk rn \newline
                                 S[ri+t] = S[rj+t] * C[rk+t] for t=0...(rn-1).  "## & ""
  MDIVC & 0x134 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MDIVC ri rj rk rn \newline
                                 C[ri+t] = C[rj+t] / C[rk+t] for t=0...(rn-1).  "## & ""
  MMODC & 0x136 & (value: r , value: r, value: r, value: r) & mem_read mem_write & r##"MMODC ri rj rk rn \newline
                                 C[ri+t] = C[rj+t] \% C[rk+t] for t=0...(rn-1).  "## & ""
  MREVC & 0x138 & (value: r, value: r, value: r) & mem_read mem_write & r##"MREVC ri rj rn \newline
                                 Reverses the array, as in C[ri+t] = C[rj+rn-1-t] for t=0...(rn-1).  "## & ""
  MREVS & 0x139 & (value: r, value: r, value: r) & mem_read mem_write & r##"MREVS ri rj rn \newline
                                 Reverses the array, as in S[ri+t] = S[rj+rn-1-t] for t=0...(rn-1).  "## & ""
  MEVALCC & 0x13A & (dest: cw, value: r, value: r, value: c) & mem_read & r##"MEVALCC ci rj rn ck \newline
                                 Evaluates the polynomial \verb|ci = sum_{t=0}^{rn-1} C[rj+t]*ck^t|. "## & ""
  MEVALSC & 0x13B & (dest: sw, value: r, value: r, value: c) & mem_read & r##"MEVALSC si rj rn ck \newline
                                 Evaluates the polynomial \verb|si = sum_{t=0}^{rn-1} S[rj+t]*ck^t|. "## & ""
  MBITDECC & 0x13C & (value: r, value: c, value: r) & mem_write & r##"MBITDECC ri cj rk \newline
                                 Takes cint register cj and decomposes it into rk bits and places them in C[ri+t] for t=0...rk-1. "## & ""
  MBITDECINT & 0x13D & (value: r, value: r, value: r) & mem_write & r##"MBITDECC ri rj rk \newline
                                 Takes register rj and decomposes it into rk bits and places them in R[ri+t] for t=0...rk-1. "## & ""
  MBITDECCS & 0x13E & (value: r, value: c, value: r) & mem_write & r##"MBITDECC ri cj rk \newline
                                 Takes cint register cj and decomposes it into rk bits and places them in C[ri+t] for t=0...rk-1. Assumes cj is signed, i.e. if cj>p/2 then this bit-decomposes cj-p "## & ""
},
}
