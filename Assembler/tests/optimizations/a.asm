ldi c0, 3
ldsi s0, 3
ldmc c1, 3
ldms s1, 3
stmc c0, 8000
stms s0, 8000
ldmci c2, r0
ldmsi s2, r0
stmci c0, r0
stmsi s0, r0
movc c3, c2
movs s3, s2
movint r1, r0
ldmint r2, 3
stmint r0, 8000
ldminti r3, r2
stminti r3, r2
pushint r3
popint r4
ldtn r1
ldarg r5
reqbl 10
starg r5
run_tape 1, 1, 2
join_tape 3
jmpne r1, 0, 1
crash
jmpne r2, 0, 1
restart
clear_memory
clear_registers
addc c6, c1, c2
adds s6, s1, s2
addm s7, s1, c2
addci c8, c1, 10
addsi s8, s1, 10
subc c9, c1, c2
subs s9, s1, s2
subml s10, s9, c1
submr s11, c1, s9
subci c10, c1, 10
subsi s12, s1, 10
subcfi c11, c1, 10
subsfi s11, s1, 10
mulc c12, c1, c2
mulm s12, s1, c2
mulci c13, c1, 10
mulsi s13, s1, 10
divc c14, c3, c2
divci c15, c3, 10
modc c16, c1, c2
modci c17, c1, 10
legendrec c18, c1
digestc c19, c2
output_clear c1, 1
input_clear c20, 1
output_share 5, 1, s1, s2, s3, s4
input_share 6, 2, s13, s14, s15, s16, s17
private_input s18, 1, 10
private_output s18, 2, 10
output_int r10, 10
input_int r11, 10
open_chan r13, 10
close_chan 10
startopen 4, s1, s2, s3, s4
stopopen 4, c1, c2, c3, c4
opensint r1, sr1
opensbit r1, sb1
triple s1,s2,s3
bit s1
square s1,s2
dabit s1, sb1
ldmsint sr1, 10
ldmsinti sr1, r1
stmsint sr1, 10
stmsinti sr1, r1
movsint sr1, sr2
ldsint sr1, 10
addsint sr1, sr2, sr3
addsintc sr1, sr2, r3
subsint sr1, sr2, sr3
subsintc sr1, sr2, r3
subcints sr1, r2, sr3
mulsint sr1, sr2, sr3
mulsintc sr1, sr2, r3
divsint sr1, sr2, sr3
shlsint sr1, sr2, 10
shrsint sr1, sr2, 10
andc c3, c1, c2
xorc c3, c1, c2
orc c3, c1, c2
andci c3, c1, 10
xorci c3, c1, 10
orci c3, c1, 10
notc c3, c1, 10
neg sr1, sr2
sand sr1, sr2, sb1
xorsb sb3, sb1, sb2
andsb sb3, sb1, sb2
orsb sb3, sb1, sb2
negb sb3, sb1
shlc c3, c1, c2
shrc c3, c1, c2
shlci c3, c1, 10
shrci c3, c1, 10
jmp 1
jmpne r1, 0, 1
jmpeq r1, 0, 1
eqzint r2, r1
ltzint r2, r1
ltint r3, r1, r2
gtint r3, r1, r2
eqint r3, r1, r2
ldint r1, 10
addint r1, r2, r3
subint r1, r2, r3
mulint r1, r2, r3
divint r1, r2, r3
convint c1, r2
convmodp r1, c2, 10
convsintsreg sr5, s1
convregsreg sr379, r0
convsregsint s0, sr0
print_mem 10
print_reg c1
print_char 10
print_char4 10
print_char_regint r1
print_char4_regint r1
print_float c1, c2, c3, c4, c5
print_fix c1, 10, 5
print_int r1
eqzsint sb1, sr1
ltzsint sb1, sr1
andsint sr1, sr2, sr3
andsintc sr1, sr2, r10
orsint sr1, sr2, sr3
orsintc sr1, sr2, r10
xorsint sr1, sr2, s3
xorsintc sr1, sr2, r10
invsint sr1, sr2
mul2sint sr1, sr2, sr3, sr4
gc 8, 10, 3, 2, sr1, sr2, sr3, sr10, sr11
bitsint sb1, sr1, 10
sintbit sr1, sr2, sb10, 10
rand r1, r2
start_clock 10
stop_clock 11
lf_cint 21, 42, 5, 4, 3, 2, 1, c11, c12, c13, c14, c15, r1, r2, r3, r4, sr1, sr2, sr3, c1, c2, s1
lf_sint 21, 42, 5, 4, 3, 2, 1, s11, s12, s13, s14, s15, r1, r2, r3, r4, sr1, sr2, sr3, c1, c2, s1
lf_regint 21, 42, 5, 4, 3, 2, 1, r11, r12, r13, r14, r15, r1, r2, r3, r4, sr1, sr2, sr3, c1, c2, s1
lf_sregint 21, 42, 5, 4, 3, 2, 1, sr11, sr12, sr13, sr14, sr15, r1, r2, r3, r4, sr1, sr2, sr3, c1, c2, s1
jmp 2
return
call -1
pushint r0 # 3
getspint r3000 # 5
peekint r5, r3000 # 8
pokeint r3000, r2 # 9
popint r4 # 10
pushsint sr0 # 3
getspsint r3000 # 5
peeksint sr5, r3000 # 8
pokesint r3000, sr2 # 9
popsint sr4 # 10
pushs s0 # 3
getsps r3000 # 5
peeks s5, r3000 # 8
pokes r3000, s2 # 9
pops s4 # 10
pushc c0 # 3
getspc r3000 # 5
peekc c5, r3000 # 8
pokec r3000, c2 # 9
popc c4 # 10
pushsbit sb0 # 3
getspsbit r3000 # 5
peeksbit sb5, r3000 # 8
pokesbit r3000, sb2 # 9
popsbit sb4 # 10
memsizes 10000
memsizec 10000
memsizeint 10000
memsizesint 10000






# We should convert all upper case opcodes to lower case as well
# to avoid pains in other things
LDI c0, 3
