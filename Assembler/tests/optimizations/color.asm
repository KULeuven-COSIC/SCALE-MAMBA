ldint r1, 1 # 2
ldint r2, 1 # 24
ldint r4, 1 # 27
ldi c6, 1
ldint r7, 5 # 32
subint r3, r1, r2 # 25
subint r5, r1, r4 # 28
stmci c6, r3 # 26
pushint r5 # 29
pushint r7 # 33
ldint r6, 1 # 31
ldint r8, 1 # 34
ldint r15, 1 # 63
jmpne r15, 0, -14 # 64
ldint r17, 1 # 66
subint r18, r8, r17 # 67
pushint r18 # 68
