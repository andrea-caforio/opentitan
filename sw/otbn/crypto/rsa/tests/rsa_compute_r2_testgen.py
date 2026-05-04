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


@testcase
def gen_compute_r2_test(seed: Optional[int] = None):
    mod_size = MOD_SIZES[random.randint(0, len(MOD_SIZES)-1)]

    # Odd modulus
    n = random.randint(2**(mod_size-1), 2**mod_size - 1)
    n |= 1

    mu = pow(-n, -1, 2**WORD_SIZE)
    k = mod_size // WORD_SIZE
    b = mod_size // 8

    r2 = pow(2**mod_size, 2, n)

    mu_bytes = int.to_bytes(mu, byteorder='little', length=32)
    k_bytes = int.to_bytes(k, byteorder='little', length=4)

    return {
        "entrypoint": "main",
        "input": {
            "regs": {
                "x31": itoa_gpr(k_bytes),
                "w30": itoa_wdr(mu_bytes),
            },
            "dmem": {
                "_rsa_data": build_dmem(b, n = n),
            }
        },
        "output": {
            "dmem": {
                "_rsa_scratch": build_scratch(b, r2 = r2),
            }
        }
    }


if __name__ == '__main__':
    gen_compute_r2_test()
