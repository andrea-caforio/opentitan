/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */

.text

inv_256:

  # We compute the following algorithm:
  #
  # w21 = y = 1
  # w22 = x
  # w23 = 2
  # for i = 0 to 7 do
  #     [w27, w26] = w21 * w22 = x * y
  #     w26 = w23 - w26 = 2 - x * y
  #     [w27, w26] = w21 * 26 = x * (2 - x * y)
  #     w21 = w26
  # endfor
  # return w21

  # w21 = y = 1
  # w22 = x
  # w23 = 2
  bn.addi w21, w31, 1
  bn.mov  w22, w20
  bn.addi w23, w31, 2

  loopi 8, 8
    # x * y
    bn.mov w24, w21
    bn.mov w25, w22
    jal x1, mul256_w24xw25

    # 2 - x * y.
    bn.sub w26, w23, w26

    # x * (2 - x * y)
    bn.mov w24, w21
    bn.mov w25, w26
    jal x1, mul256_w24xw25
    bn.mov w21, w26

  ret

mul256_w24xw25:
  bn.mulqacc.z          w24.0, w25.0,  0
  bn.mulqacc            w24.1, w25.0, 64
  bn.mulqacc.so  w26.L, w24.0, w25.1, 64
  bn.mulqacc            w24.2, w25.0,  0
  bn.mulqacc            w24.1, w25.1,  0
  bn.mulqacc            w24.0, w25.2,  0
  bn.mulqacc            w24.3, w25.0, 64
  bn.mulqacc            w24.2, w25.1, 64
  bn.mulqacc            w24.1, w25.2, 64
  bn.mulqacc.so  w26.U, w24.0, w25.3, 64
  /* bn.mulqacc            w24.3, w25.1,  0 */
  /* bn.mulqacc            w24.2, w25.2,  0 */
  /* bn.mulqacc            w24.1, w25.3,  0 */
  /* bn.mulqacc            w24.3, w25.2, 64 */
  /* bn.mulqacc.so  w27.L, w24.2, w25.3, 64 */
  /* bn.mulqacc.so  w27.U, w24.3, w25.3,  0 */
  ret
