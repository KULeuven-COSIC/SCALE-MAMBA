ldi c0, 3 # converted function call directly to asm
movc c1, c0 # write to local 0: c1
ldi c2, 42 # converted function call directly to asm
movc c3, c1 # read from local 0
addc c4, c2, c3 # converted function call directly to asm
print_reg c4 # converted function call directly to asm
