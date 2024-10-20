// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "sw/device/lib/testing/keymgr_testutils.h"
#include "sw/device/lib/testing/otbn_testutils.h"
#include "sw/device/lib/testing/ret_sram_testutils.h"
#include "sw/device/lib/testing/rstmgr_testutils.h"
#include "sw/device/lib/testing/sram_ctrl_testutils.h"
#include "sw/device/lib/testing/test_framework/ottf_main.h"
#include "sw/device/silicon_creator/lib/drivers/retention_sram.h"

#include "hw/top_earlgrey/sw/autogen/top_earlgrey.h"

static dif_keymgr_t keymgr;
static dif_kmac_t kmac;
static dif_otbn_t otbn;
static dif_rstmgr_t rstmgr;
static dif_sram_ctrl_t sram_ctrl;

enum {
  kKeymgrOutputSizeWords = 8,
  kKeymgrOutputSizeBytes = 32,

  kX2551PublicKeySizeBytes = 32,

  // The retention SRAM testutils allocate some internal data as well as a
  // number of counters; both of which should not be overwritten by this
  // test. Hence, the actual base address is offset to account for this.
  kRetSramBaseAddr = TOP_EARLGREY_RAM_RET_AON_BASE_ADDR +
                     offsetof(retention_sram_t, owner) +
                     4 * kRetSramTestutilsNumberOfCounters
};

/**
 * The key manager output register is a 2-share masked vector of
 * `kKeymgrOutputSizeWords` 32-bit registers holding either an identity or a
 * versioned software key. This object bundles the masked shares and their
 * corresponding unmasked equivalent.
 */
typedef struct keymgr_output {
  uint32_t masked[2][kKeymgrOutputSizeWords];
  uint32_t unmasked[kKeymgrOutputSizeWords];
} keymgr_output_t;

/**
 * Grouping of the three key sealing output variants that can be generated in
 * each key manager state (identity, versioned software key, sideload OTBN key).
 *
 * Note that the sideload OTBN key is not visible to software. In order to
 * run the same verification steps as for the identity and software keys, a
 * X25519 public key is generated in the OTBN and retrieved. For the sake of
 * simplicity, and only in the confines of this test, we can assume that the
 * X25519 public key and the sideload OTBN key refer to the same thing.
 */
typedef struct sealing_keys {
  keymgr_output_t identity;
  keymgr_output_t sw_key;
  uint32_t sideload_key[kX2551PublicKeySizeBytes];
} sealing_keys_t;

// Symbols of the OTBN X22519 public key generation program.
// See sw/otbn/crypto/x25519_sideload.s for the source code.
OTBN_DECLARE_APP_SYMBOLS(x25519_sideload);
OTBN_DECLARE_SYMBOL_ADDR(x25519_sideload, enc_u);
OTBN_DECLARE_SYMBOL_ADDR(x25519_sideload, enc_result);
static const otbn_app_t kOtbnAppX25519 = OTBN_APP_T_INIT(x25519_sideload);
static const otbn_addr_t kOtbnVarEncU =
    OTBN_ADDR_T_INIT(x25519_sideload, enc_u);
static const otbn_addr_t kOtbnVarEncResult =
    OTBN_ADDR_T_INIT(x25519_sideload, enc_result);

OTTF_DEFINE_TEST_CONFIG();

/**
 * Initialize the dif handles required for this test.
 */
static void init_peripheral_handles(void) {
  // The testutils initialize the key manager and KMAC handles.
  CHECK_STATUS_OK(keymgr_testutils_initialize(&keymgr, &kmac));

  CHECK_DIF_OK(dif_rstmgr_init(
      mmio_region_from_addr(TOP_EARLGREY_RSTMGR_AON_BASE_ADDR), &rstmgr));
  CHECK_DIF_OK(dif_sram_ctrl_init(
      mmio_region_from_addr(TOP_EARLGREY_SRAM_CTRL_RET_AON_REGS_BASE_ADDR),
      &sram_ctrl));
  CHECK_DIF_OK(
      dif_otbn_init(mmio_region_from_addr(TOP_EARLGREY_OTBN_BASE_ADDR), &otbn));
}

/**
 * Read the key manager output shares, unmask them and write both the masked
 * and unmasked arrays into the provided `keymgr_output_t` object.
 *
 * @param output The bundle of masked and unmasked keymgr output.
 */
static void keymgr_read_output(keymgr_output_t *output) {
  dif_keymgr_output_t scratch;
  CHECK_DIF_OK(dif_keymgr_read_output(&keymgr, &scratch));
  for (int i = 0; i < kKeymgrOutputSizeWords; i++) {
    output->masked[0][i] = scratch.value[0][i];
    output->masked[1][i] = scratch.value[1][i];
    output->unmasked[i] = scratch.value[0][i] ^ scratch.value[1][i];
  }
  return;
}

/**
 * Write a `sealing_keys_t` object into the retention SRAM at a specific offset
 * from `kRetSramBaseAddr`. The offset then is incremented by the size of the
 * written data.
 *
 * @param keys The sealing keys to be stored.
 * @param offset The offset from `kRetSramBaseAddr`.
 */
static void ret_sram_write_keys(const sealing_keys_t *keys, size_t *offset) {
  uint32_t buf[sizeof(sealing_keys_t)];
  memcpy(buf, keys, sizeof(sealing_keys_t));

  sram_ctrl_testutils_write(kRetSramBaseAddr + *offset,
                            (sram_ctrl_testutils_data_t){
                                .words = buf, .len = sizeof(sealing_keys_t)});
  *offset += sizeof(sealing_keys_t);
}

/**
 * Read a `sealing_keys_t` object from the retention SRAM at a specific offset
 * from `kRetSramBaseAddr`. The offset then is incremented by the size of the
 * read data.
 *
 * @param keys The destination of the read sealing keys.
 * @param offset The offset from `kRetSramBaseAddr`.
 */
static void ret_sram_read_keys(sealing_keys_t *keys, size_t *offset) {
  memcpy(keys, (uint8_t *)(kRetSramBaseAddr + *offset), sizeof(sealing_keys_t));
  *offset += sizeof(sealing_keys_t);
}

/**
 * Invoke the generation of a sealing identity and read it back.
 *
 * @param state_name The current key manager state string.
 * @param identity The destination of the read identity.
 */
static void derive_sealing_id(const char *state_name,
                              keymgr_output_t *identity) {
  CHECK_STATUS_OK(keymgr_testutils_generate_identity(&keymgr));
  LOG_INFO("Keymgr generated identity at %s State", state_name);

  keymgr_read_output(identity);
}

/**
 * Invoke the generation of sealing versioned software key and read it back.
 * A second generation with an invalid key version should fail.
 *
 * @param state_name The current key manager state string.
 * @param key The destination of the read software key.
 */
static void derive_sealing_sw_key(const char *state_name,
                                  keymgr_output_t *key) {
  uint32_t max_version;
  CHECK_STATUS_OK(keymgr_testutils_max_key_version_get(&keymgr, &max_version));

  dif_keymgr_versioned_key_params_t params = kKeyVersionedParams;
  params.dest = kDifKeymgrVersionedKeyDestSw;
  params.version = max_version;

  CHECK_STATUS_OK(keymgr_testutils_generate_versioned_key(&keymgr, params));
  LOG_INFO("Keymgr generated SW output at %s State", state_name);

  keymgr_read_output(key);

  // If the key version is larger than the permitted maximum version, then
  // the key generation must fail.
  params.version += 1;
  CHECK(kInternal ==
        status_err(keymgr_testutils_generate_versioned_key(&keymgr, params)));
}

/**
 * Invoke the generation of sideload OTBN key, run the X25519 OTBN program and
 * read back the resulting public key. A second generation with an invalid key
 * version should fail.
 *
 * @param state_name The current key manager state string.
 * @param The destination of the read X25519 public key.
 */
static void derive_sealing_sideload_otbn_key(
    const char *state_name, uint32_t key[kKeymgrOutputSizeWords]) {
  uint32_t max_version;
  CHECK_STATUS_OK(keymgr_testutils_max_key_version_get(&keymgr, &max_version));

  dif_keymgr_versioned_key_params_t params = kKeyVersionedParams;
  params.dest = kDifKeymgrVersionedKeyDestOtbn;
  params.version = max_version;

  CHECK_STATUS_OK(keymgr_testutils_generate_versioned_key(&keymgr, params));
  LOG_INFO("Keymgr generated HW output for Otbn at %s State", state_name);

  // Run the X25519 public key generation. For more details, see the OTBN
  // sideload test sw/device/tests/keymgr_sideload_otbn_test.c.
  CHECK_STATUS_OK(otbn_testutils_load_app(&otbn, kOtbnAppX25519));
  CHECK_DIF_OK(dif_otbn_set_ctrl_software_errs_fatal(&otbn, false));

  const uint32_t kEncodedU[8] = {
      // Montgomery u-Coordinate.
      0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  };
  CHECK_STATUS_OK(otbn_testutils_write_data(&otbn, sizeof(kEncodedU),
                                            &kEncodedU, kOtbnVarEncU));
  LOG_INFO("Starting OTBN program...");
  CHECK_DIF_OK(dif_otbn_set_ctrl_software_errs_fatal(&otbn, false));
  CHECK_STATUS_OK(otbn_testutils_execute(&otbn));
  CHECK_STATUS_OK(otbn_testutils_wait_for_done(&otbn, 0));
  CHECK_STATUS_OK(otbn_testutils_read_data(&otbn, kX2551PublicKeySizeBytes,
                                           kOtbnVarEncResult, key));

  // If the key version is larger than the permitted maximum version, then
  // the key generation must fail.
  params.version += 1;
  CHECK(kInternal ==
        status_err(keymgr_testutils_generate_versioned_key(&keymgr, params)));
}

/**
 * Derive a sealing identity, software key and sideload OTBN key. If the `write`
 * flag is set, then the keys are written at specific offset in the retention
 * SRAM. In the other case, the generated keys are compared against keys from
 * the retention SRAM (corresponding to a run before the reset). Pre- and post-
 * prefix keys should match. If some previous keys are provided from an earlier
 * key manager state, compare that the newly generated ones differ.
 *
 * @param prev_key Generated keys from a previous key manager state.
 * @param state_name The current key manager state string.
 * @param offset The offset at which to read or write in the retention SRAM.
 * @param write Indicating whether to write the new keys to the retention SRAM.
 * @param dest The destination of the newly generated sealing keys.
 */
static void derive_sealing_keys(const char *state_name,
                                const sealing_keys_t *prev_keys, size_t *offset,
                                bool write, sealing_keys_t *next_keys) {
  derive_sealing_id(state_name, &next_keys->identity);
  derive_sealing_sw_key(state_name, &next_keys->sw_key);
  derive_sealing_sideload_otbn_key(state_name, next_keys->sideload_key);

  if (prev_keys) {
    CHECK_ARRAYS_NE(prev_keys->identity.unmasked, next_keys->identity.unmasked,
                    kKeymgrOutputSizeWords);
    CHECK_ARRAYS_NE(prev_keys->sw_key.unmasked, next_keys->sw_key.unmasked,
                    kKeymgrOutputSizeWords);
    CHECK_ARRAYS_NE(prev_keys->sideload_key, next_keys->sideload_key,
                    kKeymgrOutputSizeWords);
  }

  if (write) {
    ret_sram_write_keys(next_keys, offset);
  } else {
    sealing_keys_t scratch;
    ret_sram_read_keys(&scratch, offset);

    CHECK_ARRAYS_EQ(scratch.identity.unmasked, next_keys->identity.unmasked,
                    kKeymgrOutputSizeWords);
    CHECK_ARRAYS_EQ(scratch.sw_key.unmasked, next_keys->sw_key.unmasked,
                    kKeymgrOutputSizeWords);
    CHECK_ARRAYS_EQ(scratch.sideload_key, next_keys->sideload_key,
                    kKeymgrOutputSizeWords);
  }
}

/**
 * This test implements the `chip_sw_keymgr_derive_sealing` testplan item, i.e.,
 * verifies the validity of the sealing CDI flow:
 *
 * - For each keymgr operational state: `CreatorRootKey`, `OwnerIntKey` and
 *   `OwnerKey`, this test generates a sealing identity, versioned software key
 *   and a sideload OTBN key.
 * - It checks that, whenever the key manager state is advanced into a new
 *   state, the newly generated keys differ from the keys of the previous state.
 * - It verifies that the same identities and keys are generated for a given
 *   device configuration and inputs. This is achieved in two phases: The first
 *   phase consists in generating a set of keys then writing them into their
 *   retention SRAM before performing a software reset. The second phase runs
 *   through the same key generation routine but verifies that new set of keys
 *   matches the one from the first phase.
 * - The test implicitly verifies (through the key manager dif) that their
 *   software binding register is correctly locked before the key manager state
 *   is advanced.
 * - The test additionally checks that generating a key with an invalid
 *   version number results in an error.
 *
 * Due to the extensive set of checks the testplan items requires (and the
 * reset halfway through the execution) this test has a long execution time.
 *
 * @param reset_counter Indicator of the test phase.
 */
static void test_derive_sealing(size_t reset_counter) {
  const char *state_name;

  dif_keymgr_state_t keymgr_state;
  CHECK_DIF_OK(dif_keymgr_get_state(&keymgr, &keymgr_state));

  sealing_keys_t curr_keys;
  sealing_keys_t next_keys;

  size_t offset = 0;
  const bool write = reset_counter == 0;

  switch (keymgr_state) {
    case kDifKeymgrStateCreatorRootKey:

      CHECK_STATUS_OK(keymgr_testutils_state_string_get(&keymgr, &state_name));
      LOG_INFO("Keymgr entered %s State", state_name);

      derive_sealing_keys(state_name, NULL, &offset, write, &curr_keys);

      CHECK_STATUS_OK(
          keymgr_testutils_advance_state(&keymgr, &kOwnerIntParams));

      OT_FALLTHROUGH_INTENDED;
    case kDifKeymgrStateOwnerIntermediateKey:

      CHECK_STATUS_OK(keymgr_testutils_state_string_get(&keymgr, &state_name));
      LOG_INFO("Keymgr entered %s State", state_name);

      derive_sealing_keys(state_name,
                          // If the initial key manager state is smaller than
                          // the current one. Verify that the new keys differ
                          // from the previous ones.
                          keymgr_state < kDifKeymgrStateOwnerIntermediateKey
                              ? &curr_keys
                              : NULL,
                          &offset, write, &next_keys);
      memcpy(&curr_keys, &next_keys, sizeof(sealing_keys_t));

      CHECK_STATUS_OK(
          keymgr_testutils_advance_state(&keymgr, &kOwnerRootKeyParams));

      OT_FALLTHROUGH_INTENDED;
    case kDifKeymgrStateOwnerRootKey:

      CHECK_STATUS_OK(keymgr_testutils_state_string_get(&keymgr, &state_name));
      LOG_INFO("Keymgr entered %s State", state_name);

      derive_sealing_keys(
          state_name,
          keymgr_state < kDifKeymgrStateOwnerRootKey ? &curr_keys : NULL,
          &offset, write, &next_keys);

      break;
    default:
      // Theoretically, the key manager can boot into an earlier state
      // (`kDifKeymgrStateReset` or `kDifKeymgrStateInitialized`). This is not
      // supported by the testutils and thus must not occur here.
      CHECK(0, "unexpected key manager state %u", keymgr_state);
  }
}

bool test_main(void) {
  const dif_rstmgr_reset_info_bitfield_t reset_info =
      rstmgr_testutils_reason_get();
  size_t reset_counter;

  // Reset the reset counter after the first power up.
  if (reset_info == kDifRstmgrResetInfoPor) {
    CHECK_STATUS_OK(ret_sram_testutils_counter_clear(0));
  }
  CHECK_STATUS_OK(ret_sram_testutils_counter_get(0, &reset_counter));

  init_peripheral_handles();

  test_derive_sealing(reset_counter);

  // Increment the reset counter then reset the device.
  if (reset_counter == 0) {
    CHECK_STATUS_OK(ret_sram_testutils_counter_increment(0));
    CHECK_DIF_OK(dif_rstmgr_software_device_reset(&rstmgr));
    wait_for_interrupt();
  }

  return true;
}
