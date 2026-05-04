#!/usr/bin/env python3
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

import random
import math
from typing import Optional

from shared.testgen import itoa_dmem, itoa_gpr, itoa_wdr, testcase


MOD_SIZES = [256, 512, 1024, 2048, 3072, 4096]
WORD_SIZE = 256


@testcase
def gen_modinv_f4_test(seed: Optional[int] = None):
    mod_size = MOD_SIZES[random.randint(0, len(MOD_SIZES)-1)]

    k = mod_size // WORD_SIZE
    b = mod_size // 8

    e = 2**16 + 1

    x = random.randint(2**(mod_size-1), 2**mod_size - 1)
    y = pow(x, e-2, e)


    k_bytes = int.to_bytes(k, byteorder='little', length=4)

    x_bytes = int.to_bytes(x, byteorder='little', length=b)
    y_bytes = int.to_bytes(y, byteorder='little', length=32)

    return {
        "entrypoint": "main",
        "input": {
            "regs": {
                "x31": itoa_gpr(k_bytes),
            },
            "dmem" : {
                "_modinv_f4_x": itoa_dmem(x_bytes),
            }
        },
        "output": {
            "regs": {
                "w0": itoa_wdr(y_bytes),
            }
        }
    }


if __name__ == '__main__':
    gen_modinv_f4_test()
