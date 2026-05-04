/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

/* Test that gcd(x, y) is computed correctly. */

.section .text.start

main:
  bn.xor w31, w31, w31

  la x20, _gcd_x
  la x21, _gcd_y
  jal x1, gcd

  ecall

.data
.balign 32

_gcd_x:
.zero 512
_gcd_y:
.zero 512
