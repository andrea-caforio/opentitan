/* Copyright lowRISC contributors (OpenTitan project). */
/* Licensed under the Apache License, Version 2.0, see LICENSE for details. */
/* SPDX-License-Identifier: Apache-2.0 */


.section .text.start

.set RSA_MODEXP_ENABLE_MESSAGE_BLINDING, 1

/**
 * Standalone RSA 512 decrypt
 *
 * Uses OTBN modexp bignum lib to decrypt the message from the .data segment
 * in this file with the private key contained in .data segment of this file.
 *
 * Copies the decrypted message to wide registers for comparison (starting at
 * w0). See comment at the end of the file for expected values.
 */
 main:
  /* Init all-zero register. */
  bn.xor  w31, w31, w31

  /* Load number of limbs. */
  li    x30, 2

  /* Load pointers to modulus and Montgomery constant buffers. */
  la    x16, n
  la    x18, RR

  /* Compute Montgomery constants. */
  jal      x1, modload

  /* Run exponentiation.
       dmem[r0] = dmem[r0]^dmem[d] mod dmem[n] */
  la       x23, r0
  la       x24, r1
  la       x25, r2
  la       x26, d0
  la       x27, d1
  la       x28, n
  la       x29, RR
  jal      x1, modexp

  /* copy all limbs of result to wide reg file */
  la       x21, r0
  li       x8, 0
  loop     x30, 2
    bn.lid   x8, 0(x21++)
    addi     x8, x8, 1

  ecall
