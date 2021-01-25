# test_loop-0--0
ldint r0, 5 # 0
pushint r0 # 1
# test_loop-0-begin-loop-1
popint r1 # 2
convint c0, r1 # 3
print_reg c0 # 4
print_char4 539172896 # 5
print_char4 538976288 # 6
print_char 10 # 7
ldsi s0, 1 # 8
convint c1, r1 # 9
mulm s1, s0, c1 # 10
startopen 1, s1 # 11
stopopen 1, c2 # 12
print_reg c2 # 13
triple s3, s4, s5 # 14
subs s6, s1, s3 # 15
subs s7, s1, s4 # 16
startopen 2, s6, s7 # 17
stopopen 2, c3, c4 # 18
mulm s8, s4, c3 # 19
mulm s9, s3, c4 # 20
mulc c5, c3, c4 # 21
adds s10, s5, s8 # 22
adds s11, s10, s9 # 23
addm s2, s11, c5 # 24
startopen 1, s2 # 25
stopopen 1, c6 # 26
ldint r2, 1 # 27
subint r3, r1, r2 # 28
stmci c6, r3 # 29
ldint r4, 1 # 30
subint r5, r1, r4 # 31
pushint r5 # 32
jmpne r5, 0, -32 # 33
# test_loop-0-end-loop-2
popint r6 # 34
ldint r7, 5 # 35
pushint r7 # 36
# test_loop-0-begin-loop-3
popint r8 # 37
ldint r9, 5 # 38
pushint r9 # 39
# test_loop-0-begin-loop-4
popint r10 # 40
ldsi s12, 1 # 41
convint c7, r8 # 42
mulm s13, s12, c7 # 43
ldsi s14, 1 # 44
convint c8, r10 # 45
mulm s15, s14, c8 # 46
triple s17, s18, s19 # 47
subs s20, s13, s17 # 48
subs s21, s15, s18 # 49
startopen 2, s20, s21 # 50
stopopen 2, c9, c10 # 51
mulm s22, s18, c9 # 52
mulm s23, s17, c10 # 53
mulc c11, c9, c10 # 54
adds s24, s19, s22 # 55
adds s25, s24, s23 # 56
addm s16, s25, c11 # 57
startopen 1, s16 # 58
stopopen 1, c12 # 59
ldint r11, 5 # 60
mulint r12, r8, r11 # 61
addint r13, r12, r10 # 62
stmci c12, r13 # 63
ldint r14, 1 # 64
subint r15, r10, r14 # 65
pushint r15 # 66
jmpne r15, 0, -28 # 67
# test_loop-0-end-loop-5
popint r16 # 68
ldint r17, 1 # 69
subint r18, r8, r17 # 70
pushint r18 # 71
jmpne r18, 0, -36 # 72
