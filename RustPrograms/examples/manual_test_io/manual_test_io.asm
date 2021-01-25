ldint r0, 1
output_int r0, 10 # converted function call directly to asm
private_input s0, 1, 10 # converted function call directly to asm
ldsi s1, 3543 # converted function call directly to asm
triple s3, s2, s4 # secret modp multiplication
subs s5, s1, s3 # secret modp multiplication
subs s6, s0, s2 # secret modp multiplication
startopen 2, s5, s6 # secret modp multiplication
movs s5, s0 # write to local 0
movs s0, s1 # write to local 1
stopopen 2, c0, c1 # secret modp multiplication
mulm s1, s2, c0 # secret modp multiplication
mulm s2, s3, c1 # secret modp multiplication
mulc s3, c0, c1 # secret modp multiplication
adds s1, s4, s1 # secret modp multiplication
adds s1, s1, s2 # secret modp multiplication
addm s1, s1, s3 # secret modp multiplication
startopen 1, s1 # function call: reveal
movs s1, s1 # write to local 2
stopopen 1, c0 # function call: reveal
output_clear c0, 10 # converted function call directly to asm
private_output s5, 0, 10 # converted function call directly to asm
private_output s0, 1, 10 # converted function call directly to asm
private_output s1, 2, 10 # converted function call directly to asm
input_clear c0, 10 # converted function call directly to asm
input_int r0, 10 # converted function call directly to asm
output_clear c0, 10 # converted function call directly to asm
output_int r0, 10 # converted function call directly to asm
close_channel 10 # converted function call directly to asm
