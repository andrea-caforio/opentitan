/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

/* Test that e^-1 mod lcm(p-1, q-1) is computed correctly. */

.section .text.start

main:
  bn.xor w31, w31, w31

  la x20, _modinv_f4_x
  addi x21, x31, 0
  jal x1, modexp_65535_f4

  ecall

.data
.balign 32

_modinv_f4_x:
.zero 512
