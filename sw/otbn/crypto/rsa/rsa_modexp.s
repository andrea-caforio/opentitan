/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

.text

compute_r2:

  csrrw x0, FG0, x0

  addi x20, x7, 0
  addi x21, x0, 6

  loop x31, 3
    bn.lid x0, 0(x20++)
    bn.subb w0, w31, w0
    bn.movr x21++, x0
    /* End of loop */

  /* Number of iterations. */
  slli x20, x31, 3

  loop x20, 12

    addi x21, x0, 6
    csrrw x0, FG0, x0

    /* Addition */
    loop x31, 3
      bn.movr x0, x21
      bn.addc w0, w0, w0, FG0
      bn.movr x21++, x0
      /* End of loop */

    bn.addc w0, w31, w31, FG0
    bn.movr x21, x0

    addi x23, x7, 0
    addi x24, x31, 0
    jal x1, cond_sub2
    nop
    /* End of loop */

  addi x20, x8, 0
  addi x21, x0, 6
  loop x31, 2
    bn.movr x0, x21++
    bn.sid x0, 0(x20++)
    /* End of loop */

  loopi 5, 7
    addi x20, x8, 0
    addi x21, x8, 0
    addi x22, x8, 0
    addi x23, x7, 0
    addi x24, x31, 0
    jal x1, mont
    nop
    /* End of loop */

  ret

blind:

  /* fill x1, x2, r1 with random */
  addi x20, x3, 0
  addi x21, x4, 0
  loop x31, 3
    bn.wsrr w0, URND
    bn.sid x0, 0(x20++)
    bn.sid x0, 0(x21++)
    /* End of loop */

  /* compute x1 = r^e-1 */
  loopi 16, 20
    addi x20, x3, 0
    addi x21, x3, 0
    addi x22, x3, 0
    addi x23, x7, 0
    addi x24, x31, 0
    jal x1, mont
    /* End of loop */

  /* compute x0 = A * r^e-1 */
  addi x20, x2, 0
  addi x21, x3, 0
  addi x22, x2, 0
  addi x23, x7, 0
  addi x24, x31, 0
  jal x1, mont

  /* compute r2 = A * r^e */
  addi x20, x2, 0
  addi x21, x4, 0
  addi x22, x8, 0
  addi x23, x7, 0
  addi x24, x31, 0
  jal x1, mont

  ret

unblind:

  /* compute r0 = (A * r^e-1)^d-1 * A * r^e = A^d */
  addi x20, x2, 0
  addi x21, x8, 0
  addi x22, x2, 0
  addi x23, x7, 0
  addi x24, x31, 0
  jal x1, mont

  ret
