/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

/* Test that we can blind and unblind a message. */

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

  addi x20, x0, 28
  bn.lid x20, 0(x7)
  jal x1, m0inv

  jal x1, compute_r2

  /* Map */
  addi x20, x2, 0
  addi x21, x8, 0
  addi x22, x2, 0
  addi x23, x7, 0
  addi x24, x31, 0
  jal x1, mont

  la x20, _result

  /* Unblind */
  addi x21, x2, 0
  addi x22, x20, 0
  addi x23, x7, 0
  addi x24, x31, 0
  jal x1, mont1


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

_result:
.zero 512

/*
x8 = r2
x9 = buf
*/

.section .scratchpad

_rsa_scratch:
.zero 1024
