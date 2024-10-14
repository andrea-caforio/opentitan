// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "sw/device/lib/testing/keymgr_testutils.h"
#include "sw/device/lib/testing/rstmgr_testutils.h"
#include "sw/device/lib/testing/test_framework/ottf_main.h"

#include "hw/top_earlgrey/sw/autogen/top_earlgrey.h"
#include "keymgr_regs.h"  // Generated

OTTF_DEFINE_TEST_CONFIG();

#define OUTPUT_N 8

typedef struct output {
  uint32_t masked[2][OUTPUT_N];
  uint32_t unmasked[OUTPUT_N];
} output_t;

static void read_output(const dif_keymgr_t *keymgr, output_t *output) {
  dif_keymgr_output_t scratch;
  CHECK_DIF_OK(dif_keymgr_read_output(keymgr, &scratch));
  for (int i = 0; i < OUTPUT_N; i++) {
    output->masked[0][i] = scratch.value[0][i];
    output->masked[1][i] = scratch.value[1][i];
    output->unmasked[i] = scratch.value[0][i] ^ scratch.value[1][i];
  }
  return;
}

static void derive_sealing_id(const dif_keymgr_t *keymgr,
                              const char *state_name, output_t *identity) {
  CHECK_STATUS_OK(keymgr_testutils_generate_identity(keymgr));
  LOG_INFO("Keymgr generated identity at %s State", state_name);

  read_output(keymgr, identity);

  // Repeated id generations must result in the same id being created.
  CHECK_STATUS_OK(keymgr_testutils_generate_identity(keymgr));
  LOG_INFO("Keymgr generated identity at %s State", state_name);

  output_t scratch;
  read_output(keymgr, &scratch);

  CHECK_ARRAYS_EQ(identity->unmasked, scratch.unmasked, OUTPUT_N);
}

static void derive_sealing_ver_sw_key(const dif_keymgr_t *keymgr,
                                      const char *state_name, output_t *key) {
  uint32_t max_version;
  CHECK_STATUS_OK(keymgr_testutils_max_key_version_get(keymgr, &max_version));

  dif_keymgr_versioned_key_params_t params = kKeyVersionedParams;
  params.dest = kDifKeymgrVersionedKeyDestSw;
  params.version = max_version;

  CHECK_STATUS_OK(keymgr_testutils_generate_versioned_key(keymgr, params));
  LOG_INFO("Keymgr generated SW output at %s State", state_name);

  read_output(keymgr, key);

  // Repeated key generations with the same input must result in the same key
  // being created.
  CHECK_STATUS_OK(keymgr_testutils_generate_versioned_key(keymgr, params));
  LOG_INFO("Keymgr generated SW output at %s State", state_name);

  output_t scratch;
  read_output(keymgr, &scratch);

  CHECK_ARRAYS_EQ(key->unmasked, scratch.unmasked, OUTPUT_N);

  // Changing the input parameters must also change the generated key.
  params.salt[0] ^= 0x1;
  CHECK_STATUS_OK(keymgr_testutils_generate_versioned_key(keymgr, params));
  LOG_INFO("Keymgr generated SW output at %s State", state_name);

  read_output(keymgr, &scratch);
  CHECK_ARRAYS_NE(key->unmasked, scratch.unmasked, OUTPUT_N);
  params.salt[0] ^= 0x1;

  // If the key version is larger thann the permitted maximum version, then
  // the key generation must fail.
  /* params.version += 1; */
  /* CHECK_STATUS_NOT_OK(keymgr_testutils_generate_versioned_key(keymgr, params)); */
}

static void derive_sealing_sideload_otbn_key(const dif_keymgr_t *keymgr,
                                             const char *state_name) {
  uint32_t max_version;
  CHECK_STATUS_OK(keymgr_testutils_max_key_version_get(keymgr, &max_version));

  dif_keymgr_versioned_key_params_t params = kKeyVersionedParams;
  params.dest = kDifKeymgrVersionedKeyDestOtbn;
  params.version = 17;//max_version;

  CHECK_STATUS_OK(keymgr_testutils_generate_versioned_key(keymgr, params));
  LOG_INFO("Keymgr generated HW output for Otbn at %s State", state_name);
}

bool test_main(void) {
  dif_keymgr_t keymgr;
  dif_kmac_t kmac;

  const char *state_name;

  if (kDeviceType == kDeviceSimDV || kDeviceType == kDeviceSimVerilator) {
    CHECK_STATUS_OK(keymgr_testutils_startup(&keymgr, &kmac));

    // CreatorRoot state
    CHECK_STATUS_OK(keymgr_testutils_state_string_get(&keymgr, &state_name));

    // The keymgr initialization advances the module to the CreatorRootKey
    // state and generates an identity while printing the necessary checkpoint
    // strings for the DV sequence.

    output_t creator_root_id;
    output_t creator_root_key;
    derive_sealing_id(&keymgr, state_name, &creator_root_id);
    derive_sealing_ver_sw_key(&keymgr, state_name, &creator_root_key);
    derive_sealing_sideload_otbn_key(&keymgr, state_name);

    // OwnerInt state
    CHECK_STATUS_OK(keymgr_testutils_advance_state(&keymgr, &kOwnerIntParams));
    CHECK_STATUS_OK(keymgr_testutils_state_string_get(&keymgr, &state_name));
    LOG_INFO("Keymgr entered %s State", state_name);

    output_t owner_int_id;
    output_t owner_int_key;
    derive_sealing_id(&keymgr, state_name, &owner_int_id);
    derive_sealing_ver_sw_key(&keymgr, state_name, &owner_int_key);
    derive_sealing_sideload_otbn_key(&keymgr, state_name);
    CHECK_ARRAYS_NE(creator_root_id.unmasked, owner_int_id.unmasked, OUTPUT_N);
    CHECK_ARRAYS_NE(creator_root_key.unmasked, owner_int_key.unmasked,
                    OUTPUT_N);

    // OwnerRoot state
    CHECK_STATUS_OK(
        keymgr_testutils_advance_state(&keymgr, &kOwnerRootKeyParams));
    CHECK_STATUS_OK(keymgr_testutils_state_string_get(&keymgr, &state_name));
    LOG_INFO("Keymgr entered %s State", state_name);

    output_t owner_root_id;
    output_t owner_root_key;
    derive_sealing_id(&keymgr, state_name, &owner_root_id);
    derive_sealing_ver_sw_key(&keymgr, state_name, &owner_root_key);
    derive_sealing_sideload_otbn_key(&keymgr, state_name);
    CHECK_ARRAYS_NE(owner_int_id.unmasked, owner_root_id.unmasked, OUTPUT_N);
    CHECK_ARRAYS_NE(owner_int_key.unmasked, owner_root_key.unmasked, OUTPUT_N);

  } else {
    CHECK_STATUS_OK(keymgr_testutils_initialize(&keymgr, &kmac));

    // CreatorRoot state
    CHECK_STATUS_OK(keymgr_testutils_state_string_get(&keymgr, &state_name));

    // The keymgr ROM_EXT initialization advances the module to the CreatorRoot
    // state and generates an identity while printing the necessary checkpoint
    // strings for the DV sequence.

    output_t owner_root_id;
    output_t owner_root_key;
    derive_sealing_id(&keymgr, state_name, &owner_root_id);
    derive_sealing_ver_sw_key(&keymgr, state_name, &owner_root_key);
    derive_sealing_sideload_otbn_key(&keymgr, state_name);
  }

  return true;
}
