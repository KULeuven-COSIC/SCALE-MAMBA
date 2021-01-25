ldi c0, 1 # 0
shlci c1, c0, 31 # 1
mulci c2, c1, 4 # 2
mulc c0, c2, c1 # 3
bit s658 # 4
bit s643 # 5
jmpne r42, 0, 1
restart
bit s632 # 6
bit s622 # 7
bit s602 # 8
bit s586 # 9
crash
