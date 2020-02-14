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
vaddsi 2, s15, s7, 1 # 15
triple s23, s21, s31 # 16
startopen 1, s31 # 17
stopopen 1, c15 # 18
triple s24, s22, s32 # 19
startopen 1, s32 # 20
stopopen 1, c16 # 21
triple s35, s36, s37 # 22
subs s38, s22, s35 # 23
subs s39, s23, s36 # 24
startopen 2, s38, s39 # 25
stopopen 2, c23, c24 # 26
mulm s40, s36, c23 # 27
mulm s41, s35, c24 # 28
mulc c25, c23, c24 # 29
adds s42, s37, s40 # 30
adds s43, s42, s41 # 31
addm s25, s43, c25 # 32
ldi c26, 1 # 33
divc c19, c26, c15 # 34
divc c20, c26, c16 # 35
mulm s28, s25, c19 # 36
mulm s30, s24, c20 # 37
triple s44, s45, s46 # 38
subs s47, s21, s44 # 39
subs s48, s16, s45 # 40
startopen 2, s47, s48 # 41
stopopen 2, c27, c28 # 42
mulm s49, s45, c27 # 43
mulm s50, s44, c28 # 44
mulc c29, c27, c28 # 45
adds s51, s46, s49 # 46
adds s52, s51, s50 # 47
addm s33, s52, c29 # 48
startopen 1, s33 # 49
stopopen 1, c17 # 50
triple s53, s54, s55 # 51
subs s56, s28, s53 # 52
subs s57, s15, s54 # 53
startopen 2, s56, s57 # 54
stopopen 2, c30, c31 # 55
mulm s58, s54, c30 # 56
mulm s59, s53, c31 # 57
mulc c32, c30, c31 # 58
adds s60, s55, s58 # 59
adds s61, s60, s59 # 60
addm s34, s61, c32 # 61
startopen 1, s34 # 62
stopopen 1, c18 # 63
mulc c22, c17, c18 # 64
mulm s4, s30, c22 # 65
subs s9, s4, s16 # 66
subsi s10, s16, 1 # 67
subcfi c9, c1, 1 # 68
mulm s19, s9, c9 # 69
subcfi c10, c2, 1 # 70
mulm s18, s10, c10 # 71
adds s20, s19, s18 # 72
bit s107 # 73
adds s74, s107, s107 # 74
bit s91 # 75
adds s108, s74, s91 # 76
adds s75, s108, s108 # 77
bit s92 # 78
adds s109, s75, s92 # 79
adds s76, s109, s109 # 80
bit s93 # 81
adds s110, s76, s93 # 82
adds s77, s110, s110 # 83
bit s94 # 84
adds s111, s77, s94 # 85
adds s78, s111, s111 # 86
bit s95 # 87
adds s112, s78, s95 # 88
adds s79, s112, s112 # 89
bit s96 # 90
adds s113, s79, s96 # 91
adds s80, s113, s113 # 92
bit s97 # 93
adds s114, s80, s97 # 94
adds s81, s114, s114 # 95
bit s98 # 96
adds s115, s81, s98 # 97
adds s82, s115, s115 # 98
bit s99 # 99
adds s116, s82, s99 # 100
adds s83, s116, s116 # 101
bit s100 # 102
adds s117, s83, s100 # 103
adds s84, s117, s117 # 104
bit s101 # 105
adds s118, s84, s101 # 106
adds s85, s118, s118 # 107
bit s102 # 108
adds s119, s85, s102 # 109
adds s86, s119, s119 # 110
bit s103 # 111
adds s120, s86, s103 # 112
adds s87, s120, s120 # 113
bit s104 # 114
adds s121, s87, s104 # 115
adds s88, s121, s121 # 116
bit s105 # 117
adds s122, s88, s105 # 118
adds s89, s122, s122 # 119
bit s106 # 120
adds s62, s89, s106 # 121
bit s64 # 122
movs s63, s64 # 123
mulsi s65, s62, 2 # 124
adds s67, s65, s20 # 125
adds s68, s67, s63 # 126
startopen 1, s68 # 127
stopopen 1, c33 # 128
modci c34, c33, 2 # 129
mulci c35, c34, 2 # 130
mulm s69, s64, c35 # 131
addm s70, s64, c34 # 132
subs s2, s70, s69 # 133
vstartopen 2, 1, s11 # 134
vstopopen 2, 1, c37 # 135
stmc c37, 1005 # 136
vstartopen 2, 1, s13 # 137
vstopopen 2, 1, c39 # 138
stmc c39, 1006 # 139
startopen 1, s2 # 140
stopopen 1, c41 # 141
stmc c41, 1007 # 142
reqbl 19 # 143
reqbl 19 # 144
