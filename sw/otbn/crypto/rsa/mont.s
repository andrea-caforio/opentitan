.section .text.start

/* main: */
/*   bn.xor w31, w31, w31 */

/*   /\* la x27, _x *\/ */
/*   /\* bn.lid x0, 0(x27) *\/ */

/*   /\* addi x26, x0, 6 *\/ */
/*   /\* la x27, _a *\/ */
/*   /\* loopi 17, 2 *\/ */
/*   /\*   bn.lid x26, 0(x27++) *\/ */
/*   /\*   addi x26, x26, 1 *\/ */
/*   /\*   /\\* End of loop *\\/ *\/ */

/*   /\* li x24, 16 *\/ */
/*   /\* la x25, _y *\/ */
/*   /\* li x26, 6 *\/ */
/*   /\* li x27, 6 *\/ */
/*   /\* jal x1, mul *\/ */

/*   /\* addi x28, x0, 6 *\/ */
/*   /\* la x29, _z *\/ */
/*   /\* loopi 18, 2 *\/ */
/*   /\*   bn.sid x28, 0(x29++) *\/ */
/*   /\*   addi x28, x28, 1 *\/ */
/*   /\*   /\\* End of loop *\\/ *\/ */

/*   la x20, _mont_x */
/*   la x21, _mont_y */
/*   la x22, _mont_z */
/*   la x23, _mont_n */

/*   addi x24, x0, 30 */
/*   la x25, _mont_mu */
/*   bn.lid x24, 0(x25) */

/*   addi x24, x0, 16 */

/*   jal x1, mont */


/*   /\* la x16, _mont_n *\/ */
/*   /\* la x19, _mont_x *\/ */
/*   /\* la x20, _mont_y *\/ */
/*   /\* li x30, 4 *\/ */
/*   /\* li x31, 3 *\/ */
/*   /\* li x9, 3 *\/ */
/*   /\* li x10, 4 *\/ */
/*   /\* li x11, 2 *\/ */
/*   /\* jal x1, montmul *\/ */


/*   ecall */

/* x20: x, x21: y, x22: z, x23: mod, x24: size */
mont:

  addi x25, x0, 6
  addi x26, x0, 31
  addi x27, x24, 2
  loop x27, 1
    bn.movr x25++, x26
    /* End of loop */

  loop x24, 20
    bn.lid x0, 0(x20++)

    addi x25, x21, 0
    addi x26, x0, 6
    addi x27, x0, 6
    jal x1, mul

    bn.mulqacc.z         w30.0, w6.0,  0
    bn.mulqacc           w30.1, w6.0, 64
    bn.mulqacc.so  w0.L, w30.0, w6.1, 64
    bn.mulqacc           w30.2, w6.0,  0
    bn.mulqacc           w30.1, w6.1,  0
    bn.mulqacc           w30.0, w6.2,  0
    bn.mulqacc           w30.3, w6.0, 64
    bn.mulqacc           w30.2, w6.1, 64
    bn.mulqacc           w30.1, w6.2, 64
    bn.mulqacc.so  w0.U, w30.0, w6.3, 64

    addi x25, x23, 0
    addi x26, x0, 6
    addi x27, x0, 5
    jal x1, mul
    nop
    /* End of loop */

  jal x1, cond_sub2

  addi x25, x0, 6
  loop x24, 2
    bn.sid x25, 0(x22++)
    addi x25, x25, 1
    /* End of loop */

  ret

mont1:

  addi x25, x0, 6
  addi x26, x0, 31
  addi x27, x24, 2
  loop x27, 1
    bn.movr x25++, x26
    /* End of loop */

  bn.addi w0, w31, 1

  loop x24, 19
    addi x25, x21, 0
    addi x26, x0, 6
    addi x27, x0, 6
    jal x1, mul

    bn.mulqacc.z         w30.0, w6.0,  0
    bn.mulqacc           w30.1, w6.0, 64
    bn.mulqacc.so  w0.L, w30.0, w6.1, 64
    bn.mulqacc           w30.2, w6.0,  0
    bn.mulqacc           w30.1, w6.1,  0
    bn.mulqacc           w30.0, w6.2,  0
    bn.mulqacc           w30.3, w6.0, 64
    bn.mulqacc           w30.2, w6.1, 64
    bn.mulqacc           w30.1, w6.2, 64
    bn.mulqacc.so  w0.U, w30.0, w6.3, 64

    addi x25, x23, 0
    addi x26, x0, 6
    addi x27, x0, 5
    jal x1, mul

    bn.xor w0, w0, w0
    /* End of loop */

  jal x1, cond_sub2

  addi x25, x0, 6
  loop x24, 2
    bn.sid x25, 0(x22++)
    addi x25, x25, 1
    /* End of loop */

  ret

/* w0: ai, x24: len, x25: B, x26: in, x27: out */
mul:
  /* Clear flags. */
  csrrw x0, FG0, x0
  csrrw x0, FG1, x0

  /* Zero the carry register. */
  bn.xor w4, w4, w4

  /* WDR pointer. */
  addi x28, x0, 1

  loop x24, 22
    bn.lid x28, 0(x25++)

    bn.mulqacc.z         w0.0, w1.0,  0
    bn.mulqacc           w0.1, w1.0, 64
    bn.mulqacc.so  w2.L, w0.0, w1.1, 64
    bn.mulqacc           w0.2, w1.0,  0
    bn.mulqacc           w0.1, w1.1,  0
    bn.mulqacc           w0.0, w1.2,  0
    bn.mulqacc           w0.3, w1.0, 64
    bn.mulqacc           w0.2, w1.1, 64
    bn.mulqacc           w0.1, w1.2, 64
    bn.mulqacc.so  w2.U, w0.0, w1.3, 64
    bn.mulqacc           w0.3, w1.1,  0
    bn.mulqacc           w0.2, w1.2,  0
    bn.mulqacc           w0.1, w1.3,  0
    bn.mulqacc           w0.3, w1.2, 64
    bn.mulqacc.so  w3.L, w0.2, w1.3, 64
    bn.mulqacc.so  w3.U, w0.3, w1.3,  0

    bn.add  w2, w2, w4,  FG0
    bn.addc w4, w3, w31, FG0

    bn.movr x28, x26++
    bn.addc w1, w2, w1, FG1

    bn.movr x27++, x28
    /* End of loop */

  bn.movr x28, x26++
  bn.addc w1, w1, w4, FG1
  bn.movr x27++, x28

  bn.addc w1, w31, w31, FG1
  bn.movr x27++, x28

  ret

/* /\* w6 - wk, x23: modulus, x24: k*\/ */
/* cond_sub: */

/*   /\* Pointer to the last word *\/ */
/*   addi x25, x24, 6 */
/*   bn.movr x0, x25 */
/*   bn.cmp w31, w0, FG0 */
/*   csrrs x28, FG0, x0 */

/*   /\* Pointer to the penultimate word. *\/ */
/*   addi x25, x25, -1 */
/*   bn.movr x0, x25 */

/*   addi x26, x0, 1 */
/*   slli x27, x24, 5 */
/*   add x27, x27, x23 */
/*   bn.lid x26, -32(x27) */
/*   bn.cmp w1, w0, FG0 */
/*   csrrs x29, FG0, x0 */

/*   or x28, x28, x29 */
/*   csrrw x0, FG1, x28 */

/*   addi x25, x0, 6 */
/*   addi x26, x0, 1 */

/*   csrrw x0, FG0, x0 */

/*   loop x24, 5 */
/*     bn.movr x0, x25 */
/*     bn.lid x26, 0(x23++) */
/*     bn.subb w1, w0, w1 */
/*     bn.sel w0, w1, w0, FG1.C */
/*     bn.movr x25++, x0 */
/*     /\* End of loop *\/ */

/*   ret */

/* w6 - wk, x23: modulus, x24: k*/
cond_sub2:

  csrrw x0, FG1, x0

  addi x25, x23, 0
  addi x26, x24, 1

  addi x27, x0, 6
  addi x28, x0, 1

  loop x24, 3
    bn.movr x0, x27++
    bn.lid x28, 0(x25++)
    bn.cmpb w1, w0, FG1
    /* End of loop */

  bn.movr x0, x27++
  bn.cmpb w31, w0, FG1

  csrrw x0, FG0, x0
  addi x27, x0, 6

  loop x24, 5
    bn.movr x0, x27
    bn.lid x28, 0(x23++)
    bn.subb w1, w0, w1, FG0
    bn.sel w0, w1, w0, FG1.C
    bn.movr x27++, x0
    /* End of loop */

  ret

/* x20: n, x21: r2, x22: k */
/* compute_r2: */

/*   csrrw x0, FG0, x0 */

/*   addi x20, x7, 0 */
/*   addi x21, x0, 6 */

/*   loop x31, 3 */
/*     bn.lid x0, 0(x20++) */
/*     bn.subb w0, w31, w0 */
/*     bn.movr x21++, x0 */
/*     /\* End of loop *\/ */

/*   /\* Number of iterations. *\/ */
/*   slli x20, x31, 3 */

/*   loop x20, 12 */

/*     addi x21, x0, 6 */
/*     csrrw x0, FG0, x0 */

/*     /\* Addition *\/ */
/*     loop x31, 3 */
/*       bn.movr x0, x21 */
/*       bn.addc w0, w0, w0, FG0 */
/*       bn.movr x21++, x0 */
/*       /\* End of loop *\/ */

/*     bn.addc w0, w31, w31, FG0 */
/*     bn.movr x21, x0 */

/*     addi x23, x7, 0 */
/*     addi x24, x31, 0 */
/*     jal x1, cond_sub2 */
/*     nop */
/*     /\* End of loop *\/ */

/*   addi x20, x8, 0 */
/*   addi x21, x0, 6 */
/*   loop x31, 2 */
/*     bn.movr x0, x21++ */
/*     bn.sid x0, 0(x20++) */
/*     /\* End of loop *\/ */

/*   loopi 5, 7 */
/*     addi x20, x8, 0 */
/*     addi x21, x8, 0 */
/*     addi x22, x8, 0 */
/*     addi x23, x7, 0 */
/*     addi x24, x31, 0 */
/*     jal x1, mont */
/*     nop */
/*     /\* End of loop *\/ */

/*   ret */

/* x16: in, x17: out, x18: n,  */
modexp_65537:

  /* Copy over */
  addi x20, x16, 0
  addi x21, x17, 0
  loop x31, 2
    bn.lid x0, 0(x20++)
    bn.sid x0, 0(x21++)
    /* End of loop */

  loopi 16, 7
    addi x20, x17, 0
    addi x21, x17, 0
    addi x22, x17, 0
    addi x23, x18, 0
    addi x24, x31, 0
    jal x1, mont
    nop
    /* End of loop */

  addi x20, x16, 0
  addi x21, x17, 0
  addi x22, x17, 0
  addi x23, x18, 0
  addi x24, x31, 0
  jal x1, mont

  ret

/* /\* x26: y, x27: size,  w20: x, out: w0-wk *\/ */
/* mul: */

/*   addi x21, x0, 21 */
/*   addi x22, x0, 0 */
/*   addi x24, x0, 22 */
/*   addi x25, x0, 28 */

/*   bn.xor w24, w24, w24 */

/*   loop x27, 22 */
/*     bn.lid x21, 0(x26++) */

/*     bn.mulqacc.z          w20.0, w21.0,  0 */
/*     bn.mulqacc            w20.1, w21.0, 64 */
/*     bn.mulqacc.so  w22.L, w20.0, w21.1, 64 */
/*     bn.mulqacc            w20.2, w21.0,  0 */
/*     bn.mulqacc            w20.1, w21.1,  0 */
/*     bn.mulqacc            w20.0, w21.2,  0 */
/*     bn.mulqacc            w20.3, w21.0, 64 */
/*     bn.mulqacc            w20.2, w21.1, 64 */
/*     bn.mulqacc            w20.1, w21.2, 64 */
/*     bn.mulqacc.so  w22.U, w20.0, w21.3, 64 */
/*     bn.mulqacc            w20.3, w21.1,  0 */
/*     bn.mulqacc            w20.2, w21.2,  0 */
/*     bn.mulqacc            w20.1, w21.3,  0 */
/*     bn.mulqacc            w20.3, w21.2, 64 */
/*     bn.mulqacc.so  w23.L, w20.2, w21.3, 64 */
/*     bn.mulqacc.so  w23.U, w20.3, w21.3,  0 */

/*     bn.add  w22, w22, w24, FG0 */
/*     bn.addc w24, w23, w31, FG0 */

/*     bn.movr x25, x22 */
/*     bn.addc w22, w22, w28, FG1 */
/*     /\* bn.addc w25, w25, w31, FG1 *\/ */

/*     bn.movr x22++, x24 */
/*     /\* End of loop *\/ */

/*   bn.movr x25, x22 */
/*   bn.addc w22, w24, w28, FG1 */
/*   bn.movr x22++, x24 */

/*   bn.addc w22, w31, w31, FG1 */
/*   bn.movr x22++, x24 */

/*   ret */

/* x20: x, x21: k */
modexp_65535_f4:

  jal x1, mod_f4

  /* w0: state, w1: res */
  bn.mov w0, w23
  bn.addi w1, w31, 1

  loopi 16, 6
    bn.mulqacc.wo.z w23, w0.0, w1.0, 0
    jal x1, mod_f4_35

    bn.mov w1, w23

    bn.mulqacc.wo.z w23, w0.0, w0.0, 0
    jal x1, mod_f4_35

    bn.mov w0, w23
    /* End of loop */

  bn.mov w0, w1

  ret

/**
 * Precomputation of a constant m0' for Montgomery modular arithmetic
 *
 * Word-wise Montgomery modular arithmetic requires a quantity m0' to be
 * precomputed once per modulus M. m0' is the negative of the
 * modular multiplicative inverse of the lowest limb m0 of the modulus M, in
 * the field GF(2^w), where w is the number of bits per limb. w is set to 256
 * in this subroutine.
 *
 * Returns: m0' = -m0^(-1) mod 2^256
 *          with m0 being the lowest limb of the modulus M
 *
 * This subroutine implements the Dusse-Kaliski method for computing the
 * multiplicative modular inverse when the modulus is of the form 2^k.
 * [Dus] DOI https://doi.org/10.1007/3-540-46877-3_21 section 3.2
 *       (Algorithm "Modular Inverse" on p. 235)
 *
 * Flags: When leaving this subroutine, flags of FG0 depend on a
 *        the final subtraction and can be used if needed.
 *        FG0.M, FG0.L, FG0.Z depend directly on the value of the result m0'.
 *        FG0.C is always set.
 *        FG1 is not modified in this subroutine.
 *
 * @param[in]  w28: m0, the lowest 256 bit limb of the modulus M
 * @param[in]  w31: all-zero.
 * @param[out] w30: m0', negative of inverse of m0 in GF(2^256)
 *
 * clobbered registers: w0, w1, w29
 * clobbered flag groups: FG0
 */
m0inv:
  /* w0 keeps track of loop iterations in one-hot encoding, i.e.
     w0 = 2^i in the loop body below and initialized here with w0 = 1
     It is used for both the comparison in step 4 of [Dus] and the
     addition in step 6 of [Dus] */
  bn.xor    w0, w0, w0
  bn.addi   w0, w0, 1

  /* according to [Dus] the result variable y is initialized with 1 */
  /* w29 = y_0 = 1 */
  bn.mov    w29, w0

  /* iterate over all 256 bits of m0.
     i refers to the loop cycle 0..255 in the loop body below. */
  loopi     256, 13

    /* y_i <= m*y_{i-1] */
    bn.mulqacc.z          w28.0, w29.0,  0
    bn.mulqacc            w28.1, w29.0, 64
    bn.mulqacc.so   w1.L, w28.0, w29.1, 64
    bn.mulqacc            w28.2, w29.0,  0
    bn.mulqacc            w28.1, w29.1,  0
    bn.mulqacc            w28.0, w29.2,  0
    bn.mulqacc            w28.3, w29.0, 64
    bn.mulqacc            w28.2, w29.1, 64
    bn.mulqacc            w28.1, w29.2, 64
    bn.mulqacc.so   w1.U, w28.0, w29.3, 64

  /* This checks if w1 = y_i = m0*y_(i-1) < 2^(i-1) mod 2^i
     Due to the mathematical properties it can be shown that y_i at this point,
     is either 1 or (10..0..01)_(i). Therefore, just probing the i_th bit is
     the same as the full compare. */
    bn.and    w1, w1, w0

    /* Compute
       y_i=w29 <= w1=m0*y_(i-1) < 2^(i-1) mod 2^i y_i ? : y_{i-1}+2^i : y_{i-1}
       there cannot be overlaps => or'ing is as good as adding */
    bn.or     w29, w29, w1

    /* double w0 (w0 <= w0 << 1) i.e. w0=2^i */
    bn.add    w0, w0, w0

  /* finally, compute m0' (negative of inverse)
     w29 = m0' = -(m0^-1) mod 2^256 = -y_255 = 0 - y_255 = w31 - w29 */
  bn.sub    w30, w31, w29

  ret

.data
.balign 32

/* _x: */
/* .zero 32 */
/* _y: */
/* .zero 512 */
/* _z: */
/* .zero 512 */
/* .zero 32 */
/* .zero 32 */

/* _a: */
/* .zero 512 */
/* .zero 32 */


_mont_x:
.zero 512
_mont_y:
.zero 512
_mont_z:
.zero 512
_mont_n:
.zero 512
_mont_mu:
.zero 32
