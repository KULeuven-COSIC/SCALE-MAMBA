ldi c103, 2 # 1
ldi c104, 2 # 2
ldsi s337, 2 # 3
ldsi s338, 2 # 4
vmulci 2, c109, c103, 2 # 422
vmulm 2, s343, s337, c109 # 423
stms s343, 42
stms s344, 99