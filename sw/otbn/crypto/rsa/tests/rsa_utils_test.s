/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

/* Test that the various utility functions are correct. */

.section .text.start

main:
  bn.xor w31, w31, w31

  la x20, _rsa_utils_rotate_left_x
  la x21, _rsa_utils_rotate_left_y
  addi x22, x31, 0
  jal x1, rotate_left

  la x20, _rsa_utils_shift_right_x
  la x21, _rsa_utils_shift_right_y
  addi x22, x31, 0
  jal x1, shift_right

  ecall

.data
.balign 32

_rsa_utils_rotate_left_x:
.zero 512
_rsa_utils_rotate_left_y:
.zero 512

_rsa_utils_shift_right_x:
.zero 512
_rsa_utils_shift_right_y:
.zero 512
