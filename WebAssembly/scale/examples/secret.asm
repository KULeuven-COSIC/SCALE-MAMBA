ldsi s0, 42 # converted function call directly to asm
convsintsreg sr0, s0 # converted function call directly to asm
movsint sr1, sr0 # write to local 1: sr1
private_input s1, 1, 20 # converted function call directly to asm
movs s2, s1 # write to local 0: s2
private_output s1, 2, 69 # converted function call directly to asm
movs s3, s2 # read from local 0
convsintsreg sr2, s3 # converted function call directly to asm
movsint sr3, sr2 # write to local 0: sr3
ldsi s4, 3 # converted function call directly to asm
convsintsreg sr4, s4 # converted function call directly to asm
movsint sr5, sr4 # write to local 2: sr5
subsint sr6, sr2, sr4 # converted function call directly to asm
ltzsint sb0, sr6 # converted function call directly to asm
opensbit r0, sb0 # converted function call directly to asm
jmpeqz r0, 12
movsint sr7, sr3 # read from local 0
convsregsint s5, sr7 # converted function call directly to asm
private_output s5, 2, 420 # converted function call directly to asm
movsint sr9, sr3 # read from local 0
movsint sr8, sr1 # read from local 1
addsint sr10, sr9, sr8 # converted function call directly to asm
movsint sr3, sr10 # write to local 0: sr3
movsint sr11, sr5 # read from local 2
subsint sr12, sr10, sr11 # converted function call directly to asm
ltzsint sb1, sr12 # converted function call directly to asm
opensbit r1, sb1 # converted function call directly to asm
jmpnz r1, -12
