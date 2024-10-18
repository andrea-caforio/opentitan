// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "sw/device/lib/testing/keymgr_testutils.h"
#include "sw/device/lib/testing/ret_sram_testutils.h"
#include "sw/device/lib/testing/rstmgr_testutils.h"
#include "sw/device/lib/testing/sram_ctrl_testutils.h"
#include "sw/device/lib/testing/test_framework/ottf_main.h"

#include "hw/top_earlgrey/sw/autogen/top_earlgrey.h"
#include "keymgr_regs.h"  // Generated

static dif_keymgr_t keymgr;
static dif_kmac_t kmac;
static dif_rstmgr_t rstmgr;
static dif_sram_ctrl_t sram_ctrl;

OTTF_DEFINE_TEST_CONFIG();

static void init_peripheral_handles(void) {
  CHECK_STATUS_OK(keymgr_testutils_initialize(&keymgr, &kmac));

  CHECK_DIF_OK(dif_rstmgr_init(
      mmio_region_from_addr(TOP_EARLGREY_RSTMGR_AON_BASE_ADDR), &rstmgr));
  CHECK_DIF_OK(dif_sram_ctrl_init(
      mmio_region_from_addr(TOP_EARLGREY_SRAM_CTRL_RET_AON_REGS_BASE_ADDR),
      &sram_ctrl));
}

bool test_main(void) {
  const dif_rstmgr_reset_info_bitfield_t reset_info =
      rstmgr_testutils_reason_get();
  size_t reset_counter;

  CHECK_STATUS_OK(ret_sram_testutils_counter_get(0, &reset_counter));
  LOG_INFO("/////////////////// %u", reset_counter);

  if (reset_info == kDifRstmgrResetInfoPor) {
    CHECK_STATUS_OK(ret_sram_testutils_counter_clear(0));
  }
  CHECK_STATUS_OK(ret_sram_testutils_counter_get(0, &reset_counter));

 if (reset_counter == 0) {
    CHECK_STATUS_OK(ret_sram_testutils_counter_increment(0));
    CHECK_DIF_OK(dif_rstmgr_software_device_reset(&rstmgr));
    wait_for_interrupt();
  }

  return true;
}
