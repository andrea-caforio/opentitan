/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

/* Test that we can correctly compute (2^MOD_SIZE)^2 mod n. */

.section .text.start

main:
  bn.xor w31, w31, w31

  /* Slot size. */
  slli x20, x31, 5

  la x2, _rsa_data
  add x3, x2, x20
  add x4, x3, x20
  add x5, x4, x20
  add x6, x5, x20
  add x7, x6, x20

  la x8, _rsa_scratch
  add x9, x8, x20

  jal x1, compute_r2

  ecall

/*
x2 = x0
x3 = x1
x4 = x2
x5 = d0
x6 = d1
x7 = n
*/

.data
.balign 32

_rsa_data:
.zero 3072

/*
x8 = r2
x9 = buf
*/

.section .scratchpad

_rsa_scratch:
.zero 1024
