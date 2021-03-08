
# Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
# Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

import itertools

class chain(object):
    def __init__(self, *args):
        self.args = args
    def __iter__(self):
        return itertools.chain(*self.args)
