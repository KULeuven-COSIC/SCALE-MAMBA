ldi c0, 3
ldsi s0, 3
crash
# these statements are unreachable
# FIXME: report a better error
clear_memory
clear_registers
