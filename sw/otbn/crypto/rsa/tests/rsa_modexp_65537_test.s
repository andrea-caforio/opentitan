/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

/* Test that we can correctly compute x^(2^16 + 1) mod n. */

.section .text.start

main:
  bn.xor w31, w31, w31

  la x16, _modexp_65537_x
  la x17, _modexp_65537_y
  la x18, _modexp_65537_n
  jal x1, modexp_65537

  ecall

.data
.balign 32

_modexp_65537_x:
.zero 512
_modexp_65537_y:
.zero 512
_modexp_65537_n:
.zero 512
