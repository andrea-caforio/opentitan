// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_LIB_CRYPTO_IMPL_MLDSA_H_
#define OPENTITAN_SW_DEVICE_LIB_CRYPTO_IMPL_MLDSA_H_

#include <stddef.h>
#include <stdint.h>

#include "sw/device/lib/base/hardened.h"
#include "sw/device/lib/crypto/drivers/otbn.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

enum {
  kMldsa87PublicKeyBytes = 2592,
  kMldsa87PublicKeyWords = kMldsa87PublicKeyBytes / sizeof(uint32_t),
  kMldsa87SignatureBytes = 4628,
  kMldsa87SignatureWords = kMldsa87SignatureBytes / sizeof(uint32_t),
  kMldsa87MuBytes = 32,
  kMldsa87MuWords = kMldsa87MuBytes / sizeof(uint32_t),
  kMldsa87CTildePrimeBytes = 64,
  kMldsa87CTildePrimeWords = kMldsa87CTildePrimeBytes / sizeof(uint32_t),

  kMldsa87StatusOk = 0x7baf73d2,
  kMldsa87StatusFail = 0xadf1aebd,
};

OT_WARN_UNUSED_RESULT
status_t mldsa87_verify_internal_start(const otcrypto_unblinded_key_t *public_key, const otcrypto_const_word32_buf_t *signature, const otcrypto_const_word32_buf_t *mu);

OT_WARN_UNUSED_RESULT
status_t mldsa87_verify_internal_finalize(const otcrypto_const_word32_buf_t *signature, hardened_bool_t *result);
  

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // OPENTITAN_SW_DEVICE_LIB_CRYPTO_IMPL_MLDSA_H_
