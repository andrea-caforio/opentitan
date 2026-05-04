#!/usr/bin/env python3
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

from shared.testgen import itoa_dmem


def build_dmem(b, x0 = 0, x1 = 0, x2 = 0, d0 = 0, d1 = 0, n = 0):
    value = n
    value = (value << (b * 8)) | d1
    value = (value << (b * 8)) | d0
    value = (value << (b * 8)) | x2
    value = (value << (b * 8)) | x1
    value = (value << (b * 8)) | x0
    return itoa_dmem(int.to_bytes(value, byteorder='little', length=6*b))


def build_scratch(b, r2 = 0, buf = 0):
    value = buf
    value = (value << (b * 8)) | r2
    return itoa_dmem(int.to_bytes(value, byteorder='little', length=2*b))
