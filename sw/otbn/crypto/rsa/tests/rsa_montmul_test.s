/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

/* Test that the Montgomery multiplication is correct. */

.section .text.start

main:
  bn.xor w31, w31, w31

  /* Load Montgomery constant into w30. */
  /* addi x2, x0, 30 */
  /* la x3, _montmul_mu */
  /* bn.lid x2, 0(x3) */

  /* Load k (number of 256-bit limbs) into x24. */
  /* la x2, _montmul_k */
  /* lw x24, 0(x2) */

  /* addi x24, x0, 8 */

  la x20, _montmul_x
  la x21, _montmul_y
  la x22, _montmul_z
  la x23, _montmul_n
  addi x24, x30, 0
  jal x1, mont

  ecall

.data
.balign 32

_montmul_x:
.zero 512
_montmul_y:
.zero 512
_montmul_z:
.zero 512

_montmul_n:
.zero 512
/* _montmul_mu: */
/* .zero 32 */
/* _montmul_k: */
/* .zero 32 */
