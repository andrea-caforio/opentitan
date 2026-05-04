#!/usr/bin/env python3
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

import random
import math
from typing import Optional

from shared.testgen import itoa_dmem, itoa_gpr, itoa_wdr, testcase


#MOD_SIZES = [256, 512, 1024, 1280, 1536, 1784, 2048, 2304]
MOD_SIZES = [256, 512, 1024]
WORD_SIZE = 256


@testcase
def gen_gcd_test(seed: Optional[int] = None):
    mod_size = MOD_SIZES[random.randint(0, len(MOD_SIZES)-1)]

    k = mod_size // WORD_SIZE
    b = mod_size // 8

    x = random.randint(2**(mod_size-1), 2**mod_size - 1)
    y = random.randint(2**(mod_size-1), 2**mod_size - 1)
    z = math.gcd(x, y)

    l = math.lcm(x, y)

    k_bytes = int.to_bytes(k, byteorder='little', length=4)

    x_bytes = int.to_bytes(x, byteorder='little', length=b)
    y_bytes = int.to_bytes(y, byteorder='little', length=b)
    z_bytes = int.to_bytes(z, byteorder='little', length=b)

    return {
        "entrypoint": "main",
        "input": {
            "regs": {
                "x31": itoa_gpr(k_bytes),
            },
            "dmem" : {
                "_gcd_x": itoa_dmem(x_bytes),
                "_gcd_y": itoa_dmem(y_bytes),
            }
        },
        "output": {
            "dmem": {
                "_gcd_y": itoa_dmem(z_bytes),
            }
        }
    }


if __name__ == '__main__':
    gen_gcd_test()
