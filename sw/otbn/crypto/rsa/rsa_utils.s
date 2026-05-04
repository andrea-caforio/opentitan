/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

.text

/* x20: in, x21: out, x22: k */
rotate_left:

  csrrw x0, FG0, x0

  addi x23, x21, 0

  loop x22, 3
    bn.lid x0, 0(x20++)
    bn.addc w0, w0, w0, FG0
    bn.sid x0, 0(x23++)
    /* End of loop */

  bn.lid x0, 0(x21)
  bn.addc w0, w0, w31, FG0
  bn.sid x0, 0(x21)

  ret

shift_right:

  slli x25, x22, 5

  add x24, x20, x25
  add x23, x21, x25

  bn.xor w1, w1, w1
  bn.addi w2, w31, 1

  loop x22, 7
    bn.lid x0, -32(x24)
    bn.and w3, w0, w2
    bn.rshi w0, w1, w0 >> 1
    bn.sid x0, -32(x23)
    addi x23, x23, -32
    addi x24, x24, -32
    bn.mov w1, w3
    /* End of loop */

  ret
