# if_assignment-0--0
ldint r0, 1 # 0
jmpeq r0, 0, 6 # 1
# if_assignment-0-if-block-1
print_char 84 # 2
print_char 114 # 3
print_char 117 # 4
print_char 101 # 5
print_char 10 # 6
jmp 20 # 7
# if_assignment-0-else-block-2
print_char4 1936482630 # 8
print_char 101 # 9
print_char 10 # 10
ldsi s0, 5 # 11
ldsi s1, 10 # 12
triple s3, s4, s5 # 13
subs s6, s0, s3 # 14
subs s7, s1, s4 # 15
startopen 2, s6, s7 # 16
stopopen 2, c0, c1 # 17
mulm s8, s4, c0 # 18
mulm s9, s3, c1 # 19
mulc c2, c0, c1 # 20
adds s10, s5, s8 # 21
adds s11, s10, s9 # 22
addm s2, s11, c2 # 23
startopen 1, s2 # 24
stopopen 1, c3 # 25
print_reg c3 # 26
print_char 10 # 27
