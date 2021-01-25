ldint r1, 1835104357
ldint r2, 1936026736
ldint r3, 65536
mulint r3, r3, r3
mulint r2, r2, r3
addint r1, r2, r1
stmint r1, 60
ldint r1, 1936028719
ldint r2, 1919049588
ldint r3, 65536
mulint r3, r3, r3
mulint r2, r2, r3
addint r1, r2, r1
stmint r1, 61
ldint r1, 1751346785
ldint r2, 1767992623
ldint r3, 65536
mulint r3, r3, r3
mulint r2, r2, r3
addint r1, r2, r1
stmint r1, 62
ldint r1, 1936862830
ldint r2, 1048576
ldint r3, 65536
mulint r3, r3, r3
mulint r2, r2, r3
addint r1, r2, r1
stmint r1, 63
ldint r1, 28
ldint r2, 33
ldint r3, 65536
mulint r3, r3, r3
mulint r2, r2, r3
addint r1, r2, r1
stmint r1, 64
ldint r1, 8
ldint r2, 1048576
ldint r3, 65536
mulint r3, r3, r3
mulint r2, r2, r3
addint r1, r2, r1
stmint r1, 65
ldint r1, 28
ldint r2, 34
ldint r3, 65536
mulint r3, r3, r3
mulint r2, r2, r3
addint r1, r2, r1
stmint r1, 66
ldint r1, 8
stmint r1, 67
ldint r1, 60
stmint r1, 68
ldint r1, 60
stmint r1, 69
ldint r1, 60
stmint r1, 71
ldint r1, 60
stmint r1, 70
ldmint r1, 73
ldint r2, 60
addint r1, r1, r2
stmint r1, 73
ldmint r1, 72
ldint r2, 60
addint r1, r1, r2
stmint r1, 72
ldmint r1, 75
ldint r2, 60
addint r1, r1, r2
stmint r1, 75
ldmint r1, 74
ldint r2, 60
addint r1, r1, r2
stmint r1, 74
ldsi s0, 35 # converted function call directly to asm
ldsi s1, 34 # converted function call directly to asm
stms s0, 50 # converted function call directly to asm
stms s1, 51 # converted function call directly to asm
ldmint r1, 68
ldmint r2, 64
ldint r3, 32
shrint r2, r2, r3
addint r1, r1, r2
ldint r2, 1000
addint r1, r1, r2
ldint r2, 0
stminti r2, r1 # converted function call directly to asm
ldmint r1, 68
ldmint r2, 66
ldint r3, 32
shrint r2, r2, r3
addint r1, r1, r2
ldint r2, 1000
addint r1, r1, r2
ldint r2, 1
stminti r2, r1 # converted function call directly to asm
ldi c0, 25 # converted function call directly to asm
ldms s4, 50 # converted function call directly to asm
addm s4, s4, c0 # converted function call directly to asm
startopen 1, s4 # function call: reveal
stopopen 1, c1 # function call: reveal
stmc c1, 0 # converted function call directly to asm
ldi c1, 8 # converted function call directly to asm
stmc c1, 3 # converted function call directly to asm
ldi c0, 4 # converted function call directly to asm
ldms s4, 51 # converted function call directly to asm
addm s4, s4, c0 # converted function call directly to asm
startopen 1, s4 # function call: reveal
stopopen 1, c1 # function call: reveal
stmc c1, 1 # converted function call directly to asm
ldi c1, 9 # converted function call directly to asm
stmc c1, 4 # converted function call directly to asm
ldi c1, 19 # converted function call directly to asm
stmc c1, 12 # converted function call directly to asm
ldi c0, 23 # converted function call directly to asm
ldms s4, 50 # converted function call directly to asm
mulm s4, s4, c0 # converted function call directly to asm
movs s0, s4 # write to local 0
stms s4, 10 # converted function call directly to asm
startopen 1, s0 # function call: reveal
stopopen 1, c1 # function call: reveal
stmc c1, 10 # converted function call directly to asm
startopen 1, s0 # function call: reveal
stopopen 1, c1 # function call: reveal
convmodp r0, c1, 64 # converted function call directly to asm
ldms s4, 10 # converted function call directly to asm
movs s0, s4 # write to local 0
triple s6, s5, s7 # secret modp multiplication
subs s8, s0, s6 # secret modp multiplication
subs s4, s4, s5 # secret modp multiplication
startopen 2, s8, s4 # secret modp multiplication
stopopen 2, c1, c2 # secret modp multiplication
mulm s4, s5, c1 # secret modp multiplication
mulm s5, s6, c2 # secret modp multiplication
mulc c1, c1, c2 # secret modp multiplication
adds s6, s7, s4 # secret modp multiplication
adds s4, s6, s5 # secret modp multiplication
addm s1, s4, c1 # secret modp multiplication
eqzint r1, r0
eqzint r1, r1
jmpne r1, 0, 55
jmp 42
movs s0, s2 # obtain previous block's return value
startopen 1, s2 # function call: reveal
stopopen 1, c1 # function call: reveal
stmc c1, 11 # converted function call directly to asm
ldi c0, 805 # converted function call directly to asm
ldi c0, 23 # converted function call directly to asm
ldms s4, 50 # converted function call directly to asm
mulm s4, s4, c0 # converted function call directly to asm
movs s0, s4 # write to local 0
stms s4, 10 # converted function call directly to asm
startopen 1, s0 # function call: reveal
stopopen 1, c1 # function call: reveal
convmodp r0, c1, 64 # converted function call directly to asm
ldms s4, 10 # converted function call directly to asm
movs s0, s4 # write to local 0
triple s6, s5, s7 # secret modp multiplication
subs s8, s0, s6 # secret modp multiplication
subs s4, s4, s5 # secret modp multiplication
startopen 2, s8, s4 # secret modp multiplication
stopopen 2, c1, c2 # secret modp multiplication
mulm s4, s5, c1 # secret modp multiplication
mulm s5, s6, c2 # secret modp multiplication
mulc c1, c1, c2 # secret modp multiplication
adds s6, s7, s4 # secret modp multiplication
adds s4, s6, s5 # secret modp multiplication
addm s1, s4, c1 # secret modp multiplication
eqzint r1, r0
jmpne r1, 0, 12
triple s5, s4, s6 # secret modp multiplication
subs s7, s0, s5 # secret modp multiplication
subs s8, s1, s4 # secret modp multiplication
startopen 2, s7, s8 # secret modp multiplication
stopopen 2, c1, c2 # secret modp multiplication
mulm s7, s4, c1 # secret modp multiplication
mulm s4, s5, c2 # secret modp multiplication
mulc c1, c1, c2 # secret modp multiplication
adds s5, s6, s7 # secret modp multiplication
adds s6, s5, s4 # secret modp multiplication
addm s3, s6, c1 # secret modp multiplication
jmp 16
addsi s3, s1, 11 # converted function call directly to asm
jmp 14
triple s5, s4, s6 # secret modp multiplication
subs s7, s0, s5 # secret modp multiplication
subs s8, s1, s4 # secret modp multiplication
startopen 2, s7, s8 # secret modp multiplication
stopopen 2, c1, c2 # secret modp multiplication
mulm s7, s4, c1 # secret modp multiplication
mulm s4, s5, c2 # secret modp multiplication
mulc c1, c1, c2 # secret modp multiplication
adds s5, s6, s7 # secret modp multiplication
adds s6, s5, s4 # secret modp multiplication
addm s2, s6, c1 # secret modp multiplication
jmp -54
addsi s2, s1, 11 # converted function call directly to asm
jmp -56
movs s0, s3 # obtain previous block's return value
startopen 1, s3 # function call: reveal
ldi c0, 805 # converted function call directly to asm
stopopen 1, c1 # function call: reveal
stmc c1, 12 # converted function call directly to asm
