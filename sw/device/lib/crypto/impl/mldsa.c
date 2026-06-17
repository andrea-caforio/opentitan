// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/lib/crypto/include/mldsa.h"

#include "sw/device/lib/base/hardened_memory.h"
#include "sw/device/lib/crypto/impl/status.h"
#include "sw/device/lib/crypto/include/config.h"
#include "sw/device/lib/crypto/include/datatypes.h"
#include "sw/device/lib/crypto/include/integrity.h"
#include "sw/device/lib/crypto/include/sha3.h"

#include "sw/device/lib/runtime/log.h"

#include "sw/device/lib/crypto/impl/mldsa/mldsa.h"

// Module ID for status codes.
#define MODULE_ID MAKE_MODULE_ID('m', 'l', 'd')

enum {
  kOtcryptoMldsaTrBytes = 64,
  kOtcryptoMldsaTrWords = kOtcryptoMldsaTrBytes / sizeof(uint32_t),
  kOtcryptoMldsaMuBytes = 64,
  kOtcryptoMldsaMuWords = kOtcryptoMldsaMuBytes / sizeof(uint32_t),

  kOtCryptoMldsaBufferBytes = 16384,
};

typedef struct mldsa87_hash {
  otcrypto_status_t (*hash) (const otcrypto_const_byte_buf_t, otcrypto_hash_digest_t);
  size_t digest_len; // Number of 32-bit words.
  uint8_t oid;
} mldsa87_hash_t;

static uint8_t lala[10] = {
  [kOtcryptoMldsaHashModePure & 0x1] = 10,
};

/* static const uint8_t oid[11] = { */
/*   0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x00 */
/* }; */

static uint8_t buffer[kOtCryptoMldsaBufferBytes];

/* static status_t mldsa87_pre_hash(const otcrypto_const_byte_buf_t *message, otcrypto_mldsa_hash_mode_t hash_mode, otcrypto_hash_digest_t *ph) { */

/* } */

otcrypto_status_t otcrypto_mldsa87_keygen(
    const otcrypto_unblinded_key_t *private_key,
    otcrypto_unblinded_key_t *public_key) {
  // TODO: Connect ML-DSA operations to API.
  return OTCRYPTO_NOT_IMPLEMENTED;
}

otcrypto_status_t otcrypto_mldsa87_sign(
    const otcrypto_blinded_key_t *private_key,
    const otcrypto_const_byte_buf_t message, const otcrypto_const_byte_buf_t context,
    otcrypto_mldsa_hash_mode_t hash_mode, otcrypto_word32_buf_t signature) {
  // TODO: Connect ML-DSA operations to API.
  return OTCRYPTO_NOT_IMPLEMENTED;
}

otcrypto_status_t otcrypto_mldsa87_verify(
    const otcrypto_unblinded_key_t *public_key,
    const otcrypto_const_byte_buf_t *message,
    const otcrypto_const_byte_buf_t *context,
    const otcrypto_const_word32_buf_t *signature,
    otcrypto_mldsa_hash_mode_t hash_mode,
    hardened_bool_t *verification_result) {
  HARDENED_TRY(otcrypto_mldsa87_verify_async_start(public_key, message, context, signature, hash_mode));
  return otcrypto_mldsa87_verify_async_finalize(signature, verification_result);
}

otcrypto_status_t otcrypto_mldsa87_keycheck(
    const otcrypto_unblinded_key_t *public_key,
    const otcrypto_blinded_key_t *private_key,
    hardened_bool_t *keycheck_result) {
  // TODO: Connect ML-DSA operations to API.
  return OTCRYPTO_NOT_IMPLEMENTED;
}

otcrypto_status_t otcrypto_mldsa87_keygen_async_start(
    const otcrypto_unblinded_key_t *private_key,
    otcrypto_unblinded_key_t *public_key) {
  // TODO: Connect ML-DSA operations to API.
  return OTCRYPTO_NOT_IMPLEMENTED;
}

otcrypto_status_t otcrypto_mldsa87_keygen_async_finalize(
    const otcrypto_unblinded_key_t *private_key,
    otcrypto_unblinded_key_t *public_key) {
  // TODO: Connect ML-DSA operations to API.
  return OTCRYPTO_NOT_IMPLEMENTED;
}

otcrypto_status_t otcrypto_mldsa87_sign_async_start(
    const otcrypto_blinded_key_t *private_key,
    const otcrypto_const_byte_buf_t message, const otcrypto_const_byte_buf_t context,
    otcrypto_mldsa_hash_mode_t hash_mode, otcrypto_word32_buf_t signature) {
  // TODO: Connect ML-DSA operations to API.
  return OTCRYPTO_NOT_IMPLEMENTED;
}

otcrypto_status_t otcrypto_mldsa87_sign_async_finalize(
    const otcrypto_blinded_key_t *private_key,
    const otcrypto_const_byte_buf_t message, const otcrypto_const_byte_buf_t context,
    otcrypto_mldsa_hash_mode_t hash_mode, otcrypto_word32_buf_t signature) {
  // TODO: Connect ML-DSA operations to API.
  return OTCRYPTO_NOT_IMPLEMENTED;
}

otcrypto_status_t otcrypto_mldsa87_verify_async_start(
    const otcrypto_unblinded_key_t *public_key,
    const otcrypto_const_byte_buf_t *message,
    const otcrypto_const_byte_buf_t *context, 
    const otcrypto_const_word32_buf_t *signature,
    otcrypto_mldsa_hash_mode_t hash_mode) {
  
#ifndef OTCRYPTO_DISABLE_NULL_CHECKS
  if (public_key == NULL || public_key->key == NULL ||
      (message != NULL && message->data == NULL) ||
      (context != NULL && context->data == NULL) ||
      signature == NULL || signature->data == NULL) {
    return OTCRYPTO_BAD_ARGS;
  }
#endif

  (void)lala;

  // Zeroize the buffer.
  memset(buffer, 0, kOtCryptoMldsaBufferBytes);

  LOG_INFO("1111111111111111111111111");

  /*
   * Make sure the public key, context and signature have the correct sizes.
   */

  if (public_key->key_length != kOtcryptoMldsa87PkBytes) {
    return OTCRYPTO_BAD_ARGS;
  }
  HARDENED_CHECK_EQ(public_key->key_length, kOtcryptoMldsa87PkBytes);

  if (context != NULL && context->len > kOtcryptoMldsa87ContextMaxBytes) {
    return OTCRYPTO_BAD_ARGS;
  }
  HARDENED_CHECK_NE((context == NULL || context->len <= kOtcryptoMldsa87ContextMaxBytes), 0);

  if (signature->len != kOtcryptoMldsa87SigWords) {
    return OTCRYPTO_BAD_ARGS;
  }
  HARDENED_CHECK_EQ(signature->len, kOtcryptoMldsa87SigWords);

  // TODO: Buffer consistency checks.
  // TODO: Message length check.

  /*
   * tr = SHAKE256(pk, 64).
   */

  // Convert the public key to byte buffer
  otcrypto_const_byte_buf_t pk_buf = OTCRYPTO_MAKE_BUF(otcrypto_const_byte_buf_t, (const uint8_t *) public_key->key, public_key->key_length);

  // Allocate the 64-byte tr digest.
  uint32_t tr_data[kOtcryptoMldsaTrWords] = {0};
  otcrypto_hash_digest_t tr = {
    .data = tr_data,
    .len = kOtcryptoMldsaTrWords,
  };
  HARDENED_TRY(otcrypto_shake256(&pk_buf, &tr));

  /*
   * mu = SHAKE256(tr || M', 64), where M' = 0 || len(ctx) || M.
   */

  uint8_t ctx_len = (context != NULL) ? (uint8_t) context->len : 0;

  HARDENED_TRY(randomized_bytecopy(buffer, tr.data, kOtcryptoMldsaTrBytes));
  
  buffer[kOtcryptoMldsaTrBytes] = 0;
  buffer[kOtcryptoMldsaTrBytes + 1] = ctx_len;
  if (context != NULL) {
    HARDENED_TRY(randomized_bytecopy(buffer + kOtcryptoMldsaTrBytes + 2, context->data, ctx_len));
  }
  if (message != NULL) {
    HARDENED_TRY(randomized_bytecopy(buffer + kOtcryptoMldsaTrBytes + 2 + ctx_len, message->data, message->len));
  }
    
  // Allocate the 64-byte mu digest.
  uint32_t mu_data[kOtcryptoMldsaMuWords] = {0};
  otcrypto_hash_digest_t mu = {
    .data = mu_data,
    .len = kOtcryptoMldsaMuWords,
  };

  // x
  otcrypto_const_byte_buf_t trm = OTCRYPTO_MAKE_BUF(otcrypto_const_byte_buf_t, buffer, kOtcryptoMldsaTrBytes + 2 + ctx_len + message->len);

  uint32_t *tmp = (uint32_t *)trm.data;
  for (int i = 0; i < 10; i++) {
    LOG_INFO("AAAA %08x", tmp[i]);
  }
  
  HARDENED_TRY(otcrypto_shake256(&trm, &mu));

  LOG_INFO("----------> %d", ctx_len);
  LOG_INFO("----------> %d", message->len);
  LOG_INFO("----------> %d", kOtcryptoMldsaTrBytes + 2 + ctx_len + message->len);
  LOG_INFO("----------> %08x", mu_data[0]); 

  HARDENED_TRY(mldsa87_verify_internal_start(public_key, signature, &mu));
  
  return otcrypto_eval_exit(OTCRYPTO_OK);
}

otcrypto_status_t otcrypto_mldsa87_verify_async_finalize(
    const otcrypto_const_word32_buf_t *signature,
    hardened_bool_t *verification_result) {

#ifndef OTCRYPTO_DISABLE_NULL_CHECKS
  if (signature == NULL || signature->data == NULL) {
    return OTCRYPTO_BAD_ARGS;
  }
#endif

  // TODO: Buffer integrity checks. 
  return otcrypto_eval_exit(mldsa87_verify_internal_finalize(signature, verification_result));
}

otcrypto_status_t otcrypto_mldsa87_keycheck_async_start(
    const otcrypto_unblinded_key_t *public_key,
    const otcrypto_blinded_key_t *private_key,
    hardened_bool_t *keycheck_result) {
  // TODO: Connect ML-DSA operations to API.
  return OTCRYPTO_NOT_IMPLEMENTED;
}

otcrypto_status_t otcrypto_mldsa87_keycheck_async_finalize(
    const otcrypto_unblinded_key_t *public_key,
    const otcrypto_blinded_key_t *private_key,
    hardened_bool_t *keycheck_result) {
  // TODO: Connect ML-DSA operations to API.
  return OTCRYPTO_NOT_IMPLEMENTED;
}
