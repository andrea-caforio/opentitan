/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

/* Test that the computation of the Montgomery constant is correct. */

.section .text.start

main:
  bn.xor w31, w31, w31

  jal x1, inv_256

  bn.sub w20, w31, w21

  ecall
