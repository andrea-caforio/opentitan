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
def gen_utils_test(seed: Optional[int] = None):
    mod_size = MOD_SIZES[random.randint(0, len(MOD_SIZES)-1)]

    k = mod_size // WORD_SIZE
    b = mod_size // 8

    x_rotate = random.randint(2**(mod_size-1), 2**mod_size - 1)
    x_shift = random.randint(2**(mod_size-1), 2**mod_size - 1)

    y_rotate = ((x_rotate << 1) | (x_rotate >> (mod_size - 1))) & (2**mod_size - 1)
    y_shift = x_shift >> 1

    x_rotate_bytes = int.to_bytes(x_rotate, byteorder='little', length=b)
    y_rotate_bytes = int.to_bytes(y_rotate, byteorder='little', length=b)
    x_shift_bytes = int.to_bytes(x_shift, byteorder='little', length=b)
    y_shift_bytes = int.to_bytes(y_shift, byteorder='little', length=b)

    k_bytes = int.to_bytes(k, byteorder='little', length=4)

    return {
        "entrypoint": "main",
        "input": {
            "regs": {
                "x31": itoa_gpr(k_bytes),
                 },
            "dmem": {
                "_rsa_utils_rotate_left_x": itoa_dmem(x_rotate_bytes),
                "_rsa_utils_shift_right_x": itoa_dmem(x_shift_bytes),
            }
        },
        "output": {
            "dmem": {
                "_rsa_utils_rotate_left_y": itoa_dmem(y_rotate_bytes),
                "_rsa_utils_shift_right_y": itoa_dmem(y_shift_bytes),
            }
        }
    }


if __name__ == '__main__':
    gen_utils_test()
