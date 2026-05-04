#!/usr/bin/env python3
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

import random
from typing import Optional

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
def gen_modexp_65537_test(seed: Optional[int] = None):
    mod_size = MOD_SIZES[random.randint(0, len(MOD_SIZES)-1)]

    # Odd modulus
    n = random.randint(2**(mod_size-1), 2**mod_size - 1)
    n |= 1

    r = 2**mod_size
    r2 = pow(2**mod_size, 2, n)
    mu = pow(-n, -1, 2**WORD_SIZE)
    k = mod_size // WORD_SIZE
    b = mod_size // 8

    x = random.randint(0, n - 1)
    xi = (x * r) % n

    y = pow(x, 65537, n)
    yi = (y * r) % n

    n_bytes = int.to_bytes(n, byteorder='little', length=b)
    mu_bytes = int.to_bytes(mu, byteorder='little', length=32)
    k_bytes = int.to_bytes(k, byteorder='little', length=4)

    xi_bytes = int.to_bytes(xi, byteorder='little', length=b)
    yi_bytes = int.to_bytes(yi, byteorder='little', length=b)

    return {
        "entrypoint": "main",
        "input": {
            "regs": {
                "x31": itoa_gpr(k_bytes),
                "w30": itoa_wdr(mu_bytes),
            },
            "dmem": {
                "_modexp_65537_x": itoa_dmem(xi_bytes),
                "_modexp_65537_n": itoa_dmem(n_bytes),
            }
        },
        "output": {
            "dmem": {
                "_modexp_65537_y": itoa_dmem(yi_bytes),
            }
        }
    }


if __name__ == '__main__':
    gen_modexp_65537_test()
