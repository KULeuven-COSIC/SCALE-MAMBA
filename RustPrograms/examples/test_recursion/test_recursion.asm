ldint r4, 1835104357
ldint r5, 1936026736
ldint r6, 65536
ldint r7, 1936028719
ldint r8, 1701994356
ldint r9, 65536
ldint r10, 1936880995
ldint r11, 795766633
ldint r12, 65536
ldint r13, 1852399981
ldint r14, 7565870
ldint r15, 65536
ldint r16, 1048576
ldint r17, 31
ldint r18, 65536
ldint r19, 32
ldint r20, 14
ldint r21, 65536
ldint r22, 1048576
ldint r23, 31
ldint r24, 65536
ldint r25, 33
ldint r26, 14
ldint r27, 65536
ldint r28, 1048576
ldint r29, 31
ldint r30, 65536
ldint r31, 34
ldint r32, 14
ldint r33, 65536
ldint r34, 1048576
ldint r35, 31
ldint r36, 65536
ldint r37, 35
ldint r38, 14
ldint r39, 65536
ldint r40, 1048576
ldint r41, 31
ldint r42, 65536
ldint r43, 36
ldint r44, 14
ldint r45, 65536
ldint r46, 1
ldint r47, 1
mulint r6, r6, r6
mulint r9, r9, r9
mulint r12, r12, r12
mulint r15, r15, r15
mulint r18, r18, r18
mulint r21, r21, r21
mulint r24, r24, r24
mulint r27, r27, r27
mulint r30, r30, r30
mulint r33, r33, r33
mulint r36, r36, r36
mulint r39, r39, r39
mulint r42, r42, r42
mulint r45, r45, r45
mulint r5, r5, r6
mulint r6, r8, r9
mulint r8, r11, r12
mulint r9, r14, r15
mulint r11, r17, r18
mulint r12, r20, r21
mulint r14, r23, r24
mulint r15, r26, r27
mulint r17, r29, r30
mulint r18, r32, r33
mulint r20, r35, r36
mulint r21, r38, r39
mulint r23, r41, r42
mulint r24, r44, r45
addint r4, r5, r4
addint r26, r6, r7
addint r10, r8, r10
addint r13, r9, r13
addint r11, r11, r16
addint r12, r12, r19
addint r14, r14, r22
addint r15, r15, r25
addint r19, r17, r28
addint r22, r18, r31
addint r20, r20, r34
addint r21, r21, r37
addint r23, r23, r40
addint r24, r24, r43
stmint r4, 0
stmint r26, 1
stmint r10, 2
stmint r13, 3
stmint r11, 4
stmint r12, 5
stmint r14, 6
stmint r15, 7
stmint r19, 8
stmint r22, 9
stmint r20, 10
stmint r21, 11
stmint r23, 12
stmint r24, 13
stmint r46, 15
stmint r47, 16
call 1 # call _ZN14test_recursion4main17hd927cb636a760fcfE
jmp 154 # call _ZN14test_recursion4main17hd927cb636a760fcfE
ldint r0, 0 # function call: __black_box
pushint r0
pushint r1
pushint r2
call 1 # call _ZN14test_recursion3fib17h4fa35abf76a19e7aE
jmp 7 # call _ZN14test_recursion3fib17h4fa35abf76a19e7aE
ldint r4, -1
ldint r5, 3
ldint r6, 1048688
andint r4, r0, r4 # wrap i64 to be a valid i32
shlint r4, r4, r5
addint r1, r4, r6
jmp 101 # start loop Id { idx: 1 }
popint r2
popint r1
popint r0
ldmint r4, 17
ldmint r5, 5
ldint r6, -1
ldint r7, 1
ldint r8, 65536
mulint r8, r8, r8
mulint r7, r7, r8
addint r6, r7, r6
andint r5, r5, r6
addint r4, r4, r5
ldint r5, 1000
addint r4, r4, r5
stminti r3, r4 # converted function call directly to asm
ldint r0, 1 # function call: __black_box
pushint r0
pushint r1
pushint r2
call -28 # call _ZN14test_recursion3fib17h4fa35abf76a19e7aE
popint r2
popint r1
popint r0
ldmint r4, 17
ldmint r5, 7
ldint r6, -1
ldint r7, 1
ldint r8, 65536
mulint r8, r8, r8
mulint r7, r7, r8
addint r6, r7, r6
andint r5, r5, r6
addint r4, r4, r5
ldint r5, 1000
addint r4, r4, r5
stminti r3, r4 # converted function call directly to asm
ldint r0, 2 # function call: __black_box
pushint r0
pushint r1
pushint r2
call -49 # call _ZN14test_recursion3fib17h4fa35abf76a19e7aE
popint r2
popint r1
popint r0
ldmint r4, 17
ldmint r5, 9
ldint r6, -1
ldint r7, 1
ldint r8, 65536
mulint r8, r8, r8
mulint r7, r7, r8
addint r6, r7, r6
andint r5, r5, r6
addint r4, r4, r5
ldint r5, 1000
addint r4, r4, r5
stminti r3, r4 # converted function call directly to asm
ldint r0, 3 # function call: __black_box
pushint r0
pushint r1
pushint r2
call -70 # call _ZN14test_recursion3fib17h4fa35abf76a19e7aE
popint r2
popint r1
popint r0
ldmint r4, 17
ldmint r5, 11
ldint r6, -1
ldint r7, 1
ldint r8, 65536
mulint r8, r8, r8
mulint r7, r7, r8
addint r6, r7, r6
andint r5, r5, r6
addint r4, r4, r5
ldint r5, 1000
addint r4, r4, r5
stminti r3, r4 # converted function call directly to asm
ldint r0, 4 # function call: __black_box
pushint r0
pushint r1
pushint r2
call -91 # call _ZN14test_recursion3fib17h4fa35abf76a19e7aE
popint r2
popint r1
popint r0
ldmint r4, 17
ldmint r5, 13
ldint r6, -1
ldint r7, 1
ldint r8, 65536
ldint r9, 1000
mulint r8, r8, r8
mulint r7, r7, r8
addint r6, r7, r6
andint r5, r5, r6
addint r4, r4, r5
addint r4, r4, r9
stminti r3, r4 # converted function call directly to asm
return
print_int r0 # converted function call directly to asm
print_char 10 # converted function call directly to asm
ldint r4, 3
ldint r5, 1
ldint r6, 1
ltint r7, r0, r4
ltzint r8, r0
ltzint r4, r4
eqint r9, r8, r4
subint r5, r5, r7 # negation hack, sub from 1 is the same as negation
subint r6, r6, r8 # negation hack, sub from 1 is the same as negation
andint r4, r6, r4
mulint r6, r7, r9
mulint r4, r5, r4
addint r4, r6, r4
jmpeq r4, 0, 9 # br_if
ldint r4, 8
ldint r5, 1048576
ldint r6, 0
subint r5, r1, r5
divint r4, r5, r4
addint r4, r4, r6
ldminti r4, r4
addint r3, r4, r2
return
ldint r4, -16
addint r1, r1, r4
ldint r4, -1
addint r0, r0, r4
pushint r0
pushint r1
pushint r2
call -141 # call _ZN14test_recursion3fib17h4fa35abf76a19e7aE
popint r2
popint r1
popint r0
addint r2, r3, r2
ldint r4, -2
addint r0, r0, r4
jmp -40
