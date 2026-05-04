#!/usr/bin/env python3
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

import random
from typing import Optional

from shared.testgen import itoa_wdr, testcase


MOD_SIZES = [256, 512, 1024, 2048, 3072, 4096]
WORD_SIZE = 256


@testcase
def gen_compute_mu_test(seed: Optional[int] = None):
    mod_size = MOD_SIZES[random.randint(0, len(MOD_SIZES)-1)]

    # Odd modulus
    n = random.randint(2**(mod_size-1), 2**mod_size - 1)
    n |= 1

    mu = pow(-n, -1, 2**WORD_SIZE)

    n0_bytes = int.to_bytes(n % 2**WORD_SIZE, byteorder='little', length=32)
    mu_bytes = int.to_bytes(mu, byteorder='little', length=32)

    return {
        "entrypoint": "main",
        "input": {
            "regs": {
                "w20": itoa_wdr(n0_bytes),
            }
        },
        "output": {
            "regs": {
                "w20": itoa_wdr(mu_bytes),
            }
        }
    }


if __name__ == '__main__':
    gen_compute_mu_test()
