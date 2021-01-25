ldint r0, 5 # function call: black_box
movint r1, r0 # write to local 1: r1
ldint r2, 1
ltint r3, r0, r2
ldint r4, 1
subint r5, r4, r3 # negation hack, sub from 1 is the same as negation
jmpeqz r5, 12
movint r6, r10 # read from local 0
convint c1, r6 # converted function call directly to asm
print_reg c1 # converted function call directly to asm
movint r7, r10 # read from local 0
ldint r8, 1
addint r9, r7, r8
movint r10, r9 # write to local 0: r10
movint r11, r1 # read from local 1
eqint r12, r9, r11
ldint r13, 1
subint r14, r13, r12 # negation hack, sub from 1 is the same as negation
jmpnz r14, -12
ldi c0, 42 # converted function call directly to asm
print_reg c0 # converted function call directly to asm
