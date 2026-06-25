// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/lib/base/hardened_memory.h"
#include "sw/device/lib/crypto/impl/status.h"
#include "sw/device/lib/crypto/include/config.h"
#include "sw/device/lib/crypto/include/datatypes.h"
#include "sw/device/lib/crypto/include/integrity.h"
#include "sw/device/lib/crypto/include/sha2.h"
#include "sw/device/lib/crypto/include/sha3.h"

#include "sw/device/lib/crypto/include/mldsa.h"
#include "sw/device/lib/crypto/impl/mldsa/mldsa.h"

// Module ID for status codes.
#define MODULE_ID MAKE_MODULE_ID('m', 'l', 'd')

enum {
  // Size of the truncated public-key hash tr.
  kOtcryptoMldsaTrBytes = 64,
  kOtcryptoMldsaTrWords = kOtcryptoMldsaTrBytes / sizeof(uint32_t),
  // Size of the message hash mu.
  kOtcryptoMldsaMuBytes = 64,
  kOtcryptoMldsaMuWords = kOtcryptoMldsaMuBytes / sizeof(uint32_t),
  // Size of the buffer.
  kOtCryptoMldsaBufferBytes = 16384,
  kOtCryptoMldsaBufferWords = kOtCryptoMldsaBufferBytes / sizeof(uint32_t),
  // Maximum size of a pre-hash message digest.
  kOtcryptoMldsaPhMaxWords = 16,
};

// Object identifier prefix for the pre-hash mode.
// See https://csrc.nist.gov/projects/computer-security-objects-register/algorithm-registration
static const uint8_t oid_prefix[10] = {
  0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02,
};

// ML-DSA inputs are too large to be handled on the stack. This statically
// allocated buffer is a remedy.
static uint8_t buffer[kOtCryptoMldsaBufferBytes];

/**
 * Pre-hash a message according to the `HashML-DSA` functions (Algorithms 4 and
 * 5) of FIPS-204.
 *
 * See `otcrypto_mldsa_hash_mode_t` for a list of supported hash functions.
 *
 * @param message Message to be pre-hashed.
 * @param hash_mode Selected hash mode.
 * @param ph The resulting pre-hash (maximum `kOtcryptoMldsaPhMaxWords` bytes).
 * @param oid The object identifier suffix.
 * @return Result of the operation.
 */
OT_NOINLINE OT_WARN_UNUSED_RESULT
static status_t mldsa_pre_hash(const otcrypto_const_byte_buf_t *message, otcrypto_mldsa_hash_mode_t hash_mode, otcrypto_hash_digest_t *ph, uint8_t *oid) {

  otcrypto_mldsa_hash_mode_t ph_mode = launder32(0);
  
  switch (launder32(hash_mode)) {
  case kOtcryptoMldsaHashModeSha2_256: {
    ph_mode = launder32(ph_mode) | kOtcryptoMldsaHashModeSha2_256;
    *oid = 1;
    ph->len = 8;
    HARDENED_TRY(otcrypto_sha2_256(message, ph));
  } break;
  case kOtcryptoMldsaHashModeSha2_384: {
    ph_mode = launder32(ph_mode) | kOtcryptoMldsaHashModeSha2_384;
    *oid = 2;
    ph->len = 12;
    HARDENED_TRY(otcrypto_sha2_384(message, ph));
  } break;
  case kOtcryptoMldsaHashModeSha2_512: {
    ph_mode = launder32(ph_mode) | kOtcryptoMldsaHashModeSha2_512;
    *oid = 3;
    ph->len = 16;
    HARDENED_TRY(otcrypto_sha2_512(message, ph));
  } break;
  case kOtcryptoMldsaHashModeSha3_224: {
    ph_mode = launder32(ph_mode) | kOtcryptoMldsaHashModeSha3_224;
    *oid = 7;
    ph->len = 7;
    HARDENED_TRY(otcrypto_sha3_224(message, ph));
  } break;
  case kOtcryptoMldsaHashModeSha3_256: {
    ph_mode = launder32(ph_mode) | kOtcryptoMldsaHashModeSha3_256;
    *oid = 8;
    ph->len = 8;
    HARDENED_TRY(otcrypto_sha3_256(message, ph));
  } break;
  case kOtcryptoMldsaHashModeSha3_384: {
    ph_mode = launder32(ph_mode) | kOtcryptoMldsaHashModeSha3_384;
    *oid = 9;
    ph->len = 12;
    HARDENED_TRY(otcrypto_sha3_384(message, ph));
  } break;
  case kOtcryptoMldsaHashModeSha3_512: {
    ph_mode = launder32(ph_mode) | kOtcryptoMldsaHashModeSha3_512;
    *oid = 10;
    ph->len = 16;
    HARDENED_TRY(otcrypto_sha3_512(message, ph));
  } break;
  case kOtcryptoMldsaHashModeShake128: {
    ph_mode = launder32(ph_mode) | kOtcryptoMldsaHashModeShake128;
    *oid = 11;
    ph->len = 8;
    HARDENED_TRY(otcrypto_shake128(message, ph));
  } break;
  case kOtcryptoMldsaHashModeShake256: {
    ph_mode = launder32(ph_mode) | kOtcryptoMldsaHashModeShake256;
    *oid = 12;
    ph->len = 8;
    HARDENED_TRY(otcrypto_shake256(message, ph));
  } break;
  default:
    return OTCRYPTO_BAD_ARGS;
  }
  HARDENED_CHECK_EQ(launder32(ph_mode), hash_mode);

  return OTCRYPTO_OK;
}

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

  // Scramble the buffer.
  HARDENED_TRY(hardened_memshred((uint32_t *)buffer, kOtCryptoMldsaBufferWords));

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

  /*
   * Check the integrity of public key, context, message and signature.
   */

  if (launder32(otcrypto_integrity_unblinded_key_check(public_key)) != kHardenedBoolTrue) {
    return OTCRYPTO_BAD_ARGS;
  }
  HARDENED_CHECK_EQ(otcrypto_integrity_unblinded_key_check(public_key), kHardenedBoolTrue);

  if (launder32(otcrypto_check_const_byte_buf(message)) != kHardenedBoolTrue) {
    return OTCRYPTO_BAD_ARGS;
  }
  HARDENED_CHECK_EQ(otcrypto_check_const_byte_buf(message), kHardenedBoolTrue);

  if (launder32(otcrypto_check_const_byte_buf(context)) != kHardenedBoolTrue) {
    return OTCRYPTO_BAD_ARGS;
  }
  HARDENED_CHECK_EQ(otcrypto_check_const_byte_buf(context), kHardenedBoolTrue);

  if (launder32(otcrypto_check_const_word32_buf(signature)) != kHardenedBoolTrue) {
    return OTCRYPTO_BAD_ARGS;
  }
  HARDENED_CHECK_EQ(otcrypto_check_const_word32_buf(signature), kHardenedBoolTrue);

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

  // Copy tr into the buffer.
  HARDENED_TRY(randomized_bytecopy(buffer, tr.data, kOtcryptoMldsaTrBytes));

  /*
   * mu = SHAKE256(tr || M', 64), where
   *
   *   M' = (0 || len(ctx) || ctx || msg) for pure mode.
   *   M' = (1 || len(ctx) || ctx || oid_prefix || oid_suffix || ph) for pre-hash mode.
   */

  // Context and message length in bytes.
  uint8_t ctx_len = (context != NULL) ? (uint8_t) context->len : 0;
  size_t msg_len = (message != NULL) ? message->len : 0;
  // Effective size of M'.
  size_t m_len = 0;

  if (hash_mode == kOtcryptoMldsaHashModePure) {
    // Assemble M' in the buffer.
    buffer[kOtcryptoMldsaTrBytes] = 0;
    buffer[kOtcryptoMldsaTrBytes + 1] = ctx_len;
    if (context != NULL) {
      HARDENED_TRY(randomized_bytecopy(buffer + kOtcryptoMldsaTrBytes + 2, context->data, ctx_len));
    }
    if (message != NULL) {
      HARDENED_TRY(randomized_bytecopy(buffer + kOtcryptoMldsaTrBytes + 2 + ctx_len, message->data, message->len));
    }
    m_len = kOtcryptoMldsaTrBytes + 2 + ctx_len + msg_len;
  } else {
    // Allocate the pre-hash buffer ph.
    uint32_t ph_data[kOtcryptoMldsaPhMaxWords];
    otcrypto_hash_digest_t ph = {
      .data = ph_data,
      .len = kOtcryptoMldsaPhMaxWords,
    };

    // Pre-hash the message.
    uint8_t oid_suffix;
    HARDENED_TRY(mldsa_pre_hash(message, hash_mode, &ph, &oid_suffix));

    // Assemble M' in the buffer.
    buffer[kOtcryptoMldsaTrBytes] = 1;
    buffer[kOtcryptoMldsaTrBytes + 1] = ctx_len;
    if (context != NULL) {
      HARDENED_TRY(randomized_bytecopy(buffer + kOtcryptoMldsaTrBytes + 2, context->data, ctx_len));
    }
    HARDENED_TRY(randomized_bytecopy(buffer + kOtcryptoMldsaTrBytes + 2 + ctx_len, oid_prefix, 10));
    buffer[kOtcryptoMldsaTrBytes + 2 + ctx_len + 10] = oid_suffix;
    if (message != NULL) {
      HARDENED_TRY(randomized_bytecopy(buffer + kOtcryptoMldsaTrBytes + 2 + ctx_len + 11, ph.data, ph.len << 2));
    }
    m_len = kOtcryptoMldsaTrBytes + 2 + ctx_len + 11 + (ph.len << 2);
  }

  // Allocate the 64-byte mu digest.
  uint32_t mu_data[kOtcryptoMldsaMuWords] = {0};
  otcrypto_hash_digest_t mu = {
    .data = mu_data,
    .len = kOtcryptoMldsaMuWords,
  };

  // Calculate mu.
  otcrypto_const_byte_buf_t buf = OTCRYPTO_MAKE_BUF(otcrypto_const_byte_buf_t, buffer, m_len);
  HARDENED_TRY(otcrypto_shake256(&buf, &mu));

  // Pass public key, signature and mu to the OTBN app and invoke it.
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

  // Check the integrity of the signature.
  if (launder32(otcrypto_check_const_word32_buf(signature)) != kHardenedBoolTrue) {
    return OTCRYPTO_BAD_ARGS;
  }
  HARDENED_CHECK_EQ(otcrypto_check_const_word32_buf(signature), kHardenedBoolTrue);
  
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
