# test_comparison-0--0
ldi c0, 2 # 0
ldsi s0, 1 # 1
ldsi s1, 1 # 2
movs s5, s0 # 3
movs s6, s1 # 4
modci c1, c0, 2 # 5
subc c4, c0, c1 # 6
ldi c13, 1 # 7
divci c14, c13, 2 # 8
mulc c12, c4, c14 # 9
modci c2, c12, 2 # 10
vmulci 2, c7, c1, 2 # 11
vmulm 2, s11, s5, c7 # 12
vaddm 2, s13, s5, c1 # 13
vsubs 2, s7, s13, s11 # 14
startopen 1, s14 # 15
stopopen 1, c37 # 16
stmc c37, 1005 # 17
vstartopen 2, 1, s7 # 18
vstopopen 2, 1, c38 # 19
stmc c38, 1006 # 20
reqbl 19 # 21
reqbl 19 # 22
