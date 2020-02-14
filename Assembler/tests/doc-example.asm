ldms s2, -100
ldmc c1, 10
ldms s8, 20
addm s1,s2,c1
ldmc c2, 30
addc c3,c1,c2
adds s3,s1,s2
startopen 1, s3
stopopen 1, c4
adds s4, s1, s2
addm s5, s1, c4
startopen 1, s5
stopopen 1, c6
stmc c6, 50
addm s9, s2, c2
startopen 1, s9
stopopen 1, c5
ldms s7, 110
startopen 1, s7
stopopen 1, c9
print_reg c5
startopen 1, s8
stopopen 1, c10
addc c7, c6, c10
ldmc c8, 30
mulm s6, s2, c8
