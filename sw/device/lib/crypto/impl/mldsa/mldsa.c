// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/lib/crypto/impl/mldsa/mldsa.h"

#include "sw/device/lib/base/crc32.h"
#include "sw/device/lib/base/hardened.h"
#include "sw/device/lib/base/hardened_memory.h"
#include "sw/device/lib/crypto/drivers/otbn.h"
#include "sw/device/lib/crypto/include/integrity.h"
#include "sw/device/lib/runtime/log.h"

#include "hw/top_earlgrey/sw/autogen/top_earlgrey.h"

// Module ID for status codes.
#define MODULE_ID MAKE_MODULE_ID('m', 'l', 'd')

// Declare the OTBN app.
OTBN_DECLARE_APP_SYMBOLS(mldsa87_verify);  // The ML-DSA-87 verify app.

OTBN_DECLARE_SYMBOL_ADDR(mldsa87_verify, mldsa87_verify_pk);
OTBN_DECLARE_SYMBOL_ADDR(mldsa87_verify, mldsa87_verify_sig);
OTBN_DECLARE_SYMBOL_ADDR(mldsa87_verify, mldsa87_verify_mu);

OTBN_DECLARE_SYMBOL_ADDR(mldsa87_verify, mldsa87_verify_res_ok);
OTBN_DECLARE_SYMBOL_ADDR(mldsa87_verify, mldsa87_verify_res_c_tilde_prime);

OTBN_DECLARE_SYMBOL_ADDR(mldsa87_verify, mldsa87_verify_var_c);

status_t mldsa87_verify_internal_start(const otcrypto_unblinded_key_t *public_key, const otcrypto_const_word32_buf_t *signature, const otcrypto_hash_digest_t *mu) {

  const otbn_app_t kOtbnAppMldsa87Verify = OTBN_APP_T_INIT(mldsa87_verify);
  HARDENED_TRY(otbn_load_app(kOtbnAppMldsa87Verify));

  const otbn_addr_t kOtbnPk = OTBN_ADDR_T_INIT(mldsa87_verify, mldsa87_verify_pk);
  HARDENED_TRY(otbn_dmem_write(kMldsa87PublicKeyWords, public_key->key, kOtbnPk));

  const otbn_addr_t kOtbnSig = OTBN_ADDR_T_INIT(mldsa87_verify, mldsa87_verify_sig);
  HARDENED_TRY(otbn_dmem_write(kMldsa87SignatureWords, signature->data, kOtbnSig));

  const otbn_addr_t kOtbnMu = OTBN_ADDR_T_INIT(mldsa87_verify, mldsa87_verify_mu);
  HARDENED_TRY(otbn_dmem_write(kMldsa87MuWords, mu->data, kOtbnMu));

  return otbn_execute();
}

status_t mldsa87_verify_internal_finalize(const otcrypto_const_word32_buf_t *signature, hardened_bool_t *result) {
  HARDENED_TRY(otbn_busy_wait_for_done());

  *result = kHardenedBoolFalse;

  uint32_t ok;
  const otbn_addr_t kOtbnOk = OTBN_ADDR_T_INIT(mldsa87_verify, mldsa87_verify_res_ok);
  HARDENED_TRY(otbn_dmem_read(1, kOtbnOk, &ok));
  if (launder32(ok) != kMldsa87StatusOk) {
    HARDENED_TRY(otbn_dmem_sec_wipe());
    return OTCRYPTO_BAD_ARGS;
  }
  HARDENED_CHECK_EQ(ok, kMldsa87StatusOk);

  // Read c_tilde_prime
  uint32_t c_tilde_prime[kMldsa87CTildePrimeWords];
  const otbn_addr_t kOtbnCTildePrime = OTBN_ADDR_T_INIT(mldsa87_verify, mldsa87_verify_res_c_tilde_prime);
  HARDENED_TRY(otbn_dmem_read(16, kOtbnCTildePrime, c_tilde_prime));
  LOG_INFO("MMMMMMMMMMMMMMMMm %08x", c_tilde_prime[0]);
  LOG_INFO("MMMMMMMMMMMMMMMMm %08x", c_tilde_prime[15]);
  LOG_INFO("MMMMMMMMMMMMMMMMm %08x", signature->data[0]);
  LOG_INFO("MMMMMMMMMMMMMMMMm %08x", signature->data[15]);

  *result = hardened_memeq(signature->data, c_tilde_prime, kMldsa87CTildePrimeWords);

  uint32_t mu[16];
  const otbn_addr_t kMu = OTBN_ADDR_T_INIT(mldsa87_verify, mldsa87_verify_mu);
  HARDENED_TRY(otbn_dmem_read(16, kMu, mu));

  /* LOG_INFO("MMMMMMMMMMMMMMMMm %08x", mu[0]); */
  /* LOG_INFO("MMMMMMMMMMMMMMMMm %08x", mu[15]); */

  /* LOG_INFO("MMMMMMMMMMMMMMMMMMMMM %d", otbn_instruction_count_get()); */


  /* uint32_t c[256]; */
  /* const otbn_addr_t kC = OTBN_ADDR_T_INIT(mldsa87_verify, mldsa87_verify_var_c); */
  /* HARDENED_TRY(otbn_dmem_read(256, kC, c)); */
  /* for (uint32_t i = 128; i < 256; i++) { */
  /*   LOG_INFO("---- %08x", c[i]); */
  /* } */

  return otbn_dmem_sec_wipe();
}

