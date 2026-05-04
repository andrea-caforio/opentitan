#!/usr/bin/env python3
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

import random
from typing import Optional

from rsa_utils import build_dmem, build_scratch
from shared.testgen import itoa_dmem, itoa_gpr, itoa_wdr, testcase


MOD_SIZES = [256, 512, 1024, 2048, 3072, 4096]
WORD_SIZE = 256


def mont(a, b, mu, n, mod_size):
    c = 0
    for i in range(mod_size // WORD_SIZE):
        ai = (a >> WORD_SIZE*i) % 2**WORD_SIZE
        c = c + ai * b
        d = (mu * c) % 2**WORD_SIZE
        c = (c + d * n) >> WORD_SIZE
    if c >= n:
        c -= n
    return c


@testcase
def gen_blinding_test(seed: Optional[int] = None):
    mod_size = MOD_SIZES[random.randint(0, len(MOD_SIZES)-1)]

    # Odd modulus
    n = random.randint(2**(mod_size-1), 2**mod_size - 1)
    n |= 1

    mu = pow(-n, -1, 2**WORD_SIZE)
    k = mod_size // WORD_SIZE
    b = mod_size // 8

    r2 = pow(2**mod_size, 2, n)

    x = random.randint(0, n-1)
    y = mont(x, r2, mu, n, mod_size)
    z = mont(y, 1, mu, n, mod_size)

    assert x == z

    mu_bytes = int.to_bytes(mu, byteorder='little', length=32)
    k_bytes = int.to_bytes(k, byteorder='little', length=4)

    z_bytes = int.to_bytes(z, byteorder='little', length=b)

    return {
        "entrypoint": "main",
        "input": {
            "regs": {
                "x31": itoa_gpr(k_bytes),
                 },
            "dmem": {
                "_rsa_data": build_dmem(b, x0 = x, n = n),
            }
        },
        "output": {
            "dmem": {
                "_result": itoa_dmem(z_bytes),
            }
        }
    }


if __name__ == '__main__':
    gen_blinding_test()
