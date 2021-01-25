ldint r5, 10
ldint r6, 10
ldint r7, 10
ldint r8, 10
ldint r9, 10
ldint r10, 10
ldint r11, 10
stmint r5, 10
stmint r6, 11
stmint r7, 13
stmint r8, 12
ldmint r5, 14
addint r6, r5, r9
stmint r6, 14
ldmint r7, 17
addint r8, r7, r10
stmint r8, 17
ldmint r5, 16
addint r6, r5, r11
stmint r6, 16
ldmint r7, 15
ldint r8, 10
ldint r5, 1
ldint r6, 1
ldint r9, 1
movint r0, r7 # write to local 0
ltzint r10, r7
addint r8, r0, r8
movint r1, r8 # write to local 1
gtint r7, r7, r8
ltzint r8, r8
subint r5, r5, r7 # negation hack, sub from 1 is the same as negation
eqint r7, r10, r8
subint r8, r9, r8 # negation hack, sub from 1 is the same as negation
subint r6, r6, r5 # negation hack, sub from 1 is the same as negation
andint r8, r10, r8
mulint r5, r5, r7
mulint r6, r6, r8
addint r5, r5, r6
jmpne r5, 0, 69
jmp 24
print_char 84 # converted function call directly to asm
print_char 104 # converted function call directly to asm
print_char 101 # converted function call directly to asm
print_char 114 # converted function call directly to asm
print_char 101 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 119 # converted function call directly to asm
print_char 101 # converted function call directly to asm
print_char 114 # converted function call directly to asm
print_char 101 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 101 # converted function call directly to asm
print_char 114 # converted function call directly to asm
print_char 114 # converted function call directly to asm
print_char 111 # converted function call directly to asm
print_char 114 # converted function call directly to asm
print_char 115 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 97 # converted function call directly to asm
print_char 98 # converted function call directly to asm
print_char 111 # converted function call directly to asm
print_char 118 # converted function call directly to asm
print_char 101 # converted function call directly to asm
crash
print_char 111 # converted function call directly to asm
print_char 110 # converted function call directly to asm
print_char 108 # converted function call directly to asm
print_char 121 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_int r1 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 109 # converted function call directly to asm
print_char 101 # converted function call directly to asm
print_char 109 # converted function call directly to asm
print_char 111 # converted function call directly to asm
print_char 114 # converted function call directly to asm
print_char 121 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 108 # converted function call directly to asm
print_char 101 # converted function call directly to asm
print_char 102 # converted function call directly to asm
print_char 116 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 105 # converted function call directly to asm
print_char 110 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 105 # converted function call directly to asm
print_char 54 # converted function call directly to asm
print_char 52 # converted function call directly to asm
print_char 44 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 98 # converted function call directly to asm
print_char 117 # converted function call directly to asm
print_char 116 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 114 # converted function call directly to asm
print_char 101 # converted function call directly to asm
print_char 113 # converted function call directly to asm
print_char 117 # converted function call directly to asm
print_char 101 # converted function call directly to asm
print_char 115 # converted function call directly to asm
print_char 116 # converted function call directly to asm
print_char 101 # converted function call directly to asm
print_char 100 # converted function call directly to asm
print_char 32 # converted function call directly to asm
ldint r5, 10
print_int r5 # converted function call directly to asm
crash
ldint r5, 0
ldint r6, 1
ldint r7, 1
ldint r8, 2
ldint r9, 2
ldint r10, 3
ldint r11, 3
ldint r12, 4
ldint r13, 4
ldint r14, 5
ldint r15, 5
ldint r16, 6
ldint r17, 6
ldint r18, 7
ldint r19, 7
ldint r20, 8
ldint r21, 8
ldint r22, 9
ldint r23, 9
ldint r1, 0 # write to local 1
stminti r5, r0 # converted function call directly to asm
addint r5, r0, r6
addint r6, r0, r8
addint r8, r0, r10
addint r10, r0, r12
addint r12, r0, r14
addint r14, r0, r16
addint r16, r0, r18
addint r18, r0, r20
addint r20, r0, r22
stminti r7, r5 # converted function call directly to asm
stminti r9, r6 # converted function call directly to asm
stminti r11, r8 # converted function call directly to asm
stminti r13, r10 # converted function call directly to asm
stminti r15, r12 # converted function call directly to asm
stminti r17, r14 # converted function call directly to asm
stminti r19, r16 # converted function call directly to asm
stminti r21, r18 # converted function call directly to asm
stminti r23, r20 # converted function call directly to asm
print_int r1 # converted function call directly to asm
print_char 44 # converted function call directly to asm
ldint r5, 0
print_int r5 # converted function call directly to asm
print_char 58 # converted function call directly to asm
print_char 32 # converted function call directly to asm
ldint r5, 1
shlint r5, r1, r5
movint r3, r5 # write to local 3
addint r5, r5, r0
movint r4, r5 # write to local 4
ldminti r5, r5 # converted function call directly to asm
print_int r5 # converted function call directly to asm
print_char 10 # converted function call directly to asm
ldint r5, 1
ldminti r6, r4 # converted function call directly to asm
eqint r6, r6, r3
subint r5, r5, r6 # negation hack, sub from 1 is the same as negation
jmpne r5, 0, 20
jmp 18
print_int r1 # converted function call directly to asm
print_char 44 # converted function call directly to asm
ldint r5, 1
print_int r5 # converted function call directly to asm
print_char 58 # converted function call directly to asm
print_char 32 # converted function call directly to asm
ldint r5, 1
orint r5, r3, r5
movint r3, r5 # write to local 3
addint r5, r5, r0
movint r4, r5 # write to local 4
ldminti r5, r5 # converted function call directly to asm
print_int r5 # converted function call directly to asm
print_char 10 # converted function call directly to asm
ldminti r5, r4 # converted function call directly to asm
eqint r5, r5, r3
jmpne r5, 0, 56
jmp 26
jmp -19
print_char 69 # converted function call directly to asm
print_char 82 # converted function call directly to asm
print_char 82 # converted function call directly to asm
print_char 79 # converted function call directly to asm
print_char 82 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 10 # converted function call directly to asm
ldint r2, 1 # write to local 2
jmp -44
print_char 69 # converted function call directly to asm
print_char 82 # converted function call directly to asm
print_char 82 # converted function call directly to asm
print_char 79 # converted function call directly to asm
print_char 82 # converted function call directly to asm
print_char 32 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 94 # converted function call directly to asm
print_char 10 # converted function call directly to asm
ldint r2, 1 # write to local 2
ldint r5, 1
addint r5, r1, r5
movint r1, r5 # write to local 1
jmpne r5, 5, -92 # br_if
jmp 5 # br_if
ldint r5, 1
addint r5, r1, r5
movint r1, r5 # write to local 1
jmpne r5, 5, -97 # br_if
jmp 1 # br_if
jmp -206
eqzint r5, r2
jmpne r5, 0, 1 # br_if
jmp -209
