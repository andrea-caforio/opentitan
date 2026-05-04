#!/usr/bin/env python3
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

import argparse
import random
from typing import TextIO, Optional

import sympy

from shared.testgen import write_test_data, write_test_exp, write_test_hjson

OPERAND_LIMBS = 2
LIMB_NBYTES = 32


def gen_mont_test(seed: Optional[int], hjson_file: TextIO):
    # Generate random operands.
    if seed is not None:
        random.seed(seed)

    MOD_SIZE = 4096
    PRIME_SIZE = MOD_SIZE // 2
    WORD_SIZE = 256

    p = sympy.randprime(2**(PRIME_SIZE-1), 2**PRIME_SIZE)
    q = sympy.randprime(2**(PRIME_SIZE-1), 2**PRIME_SIZE)

    n = p * q

    mu = pow(-n, -1, 2**WORD_SIZE)
    r2 = pow(2**MOD_SIZE, 2, n)

    def mont(a, b):
        c = 0
        for i in range(MOD_SIZE // WORD_SIZE):
            ai = (a >> WORD_SIZE*i) % 2**WORD_SIZE
            c = c + ai * b
            d = (mu * c) % 2**WORD_SIZE
            c = (c + d * n) >> WORD_SIZE
        # if c >= 2^MOD_SIZE:
        #     c -= n
        if c >= n:
            c -= n
        return c

    a = random.getrandbits(MOD_SIZE)
    b = random.getrandbits(MOD_SIZE)
    c = mont(a, b)

    write_test_hjson({}, {},
                     {"_mont_x": int.to_bytes(a, byteorder='little', length=MOD_SIZE//8),
                      "_mont_y": int.to_bytes(b, byteorder='little', length=MOD_SIZE//8),
                      "_mont_n": int.to_bytes(n, byteorder='little', length=MOD_SIZE//8),
                      "_mont_mu": int.to_bytes(mu, byteorder='little', length=32)},
                     {"_mont_z": int.to_bytes(c, byteorder='little', length=MOD_SIZE//8)}, hjson_file)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--seed',
                        type=int,
                        required=False,
                        help=('Seed value for pseudorandomness.'))
    parser.add_argument('hjson',
                        metavar='FILE',
                        type=argparse.FileType('w'),
                        help=('Output file for expected register values.'))
    args = parser.parse_args()

    gen_mont_test(args.seed, args.hjson)
