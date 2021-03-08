
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

from collections import defaultdict

#INIT_REG_MAX = 655360
INIT_REG_MAX = 1310720
REG_MAX = 2 ** 32
USER_MEM = 8192
TMP_MEM = 8192
TMP_MEM_BASE = USER_MEM
TMP_REG = 3
TMP_REG_BASE = REG_MAX - TMP_REG

try:
    from config_mine import *
except:
    pass
