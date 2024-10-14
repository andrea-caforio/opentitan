// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

class chip_sw_keymgr_derive_sealing_vseq extends chip_sw_keymgr_key_derivation_vseq;
  `uvm_object_utils(chip_sw_keymgr_derive_sealing_vseq)

  `uvm_object_new

  virtual task body();
    string path_internal_key = "tb.dut.top_earlgrey.u_keymgr.u_ctrl.key_o.key";
    string path_otbn_key = "tb.dut.top_earlgrey.u_keymgr.otbn_key_o";
    key_shares_t new_key;
    bit [keymgr_pkg::KeyWidth-1:0] cur_unmasked_key;
    bit [keymgr_pkg::KeyWidth-1:0] new_unmasked_key;
    bit [kmac_pkg::AppDigestW-1:0] cur_otbn_key;
    bit [kmac_pkg::AppDigestW-1:0] new_otbn_key;
    bit [kmac_pkg::AppDigestW-1:0] scratch_key;
    bit [keymgr_pkg::AdvDataWidth-1:0] creator_data;

    initialize();

    // CreatorRootKey
    `DV_WAIT(cfg.sw_logger_vif.printed_log == "Keymgr entered CreatorRootKey State",
             "Timed out waiting for keymgr to enter CreatorRootKey state",
             /*timeout_ns=*/20_000_000)
    cur_unmasked_key = get_unmasked_key(get_otp_root_key());
    `DV_CHECK_FATAL(uvm_hdl_check_path(path_internal_key))
    `DV_CHECK_FATAL(uvm_hdl_read(path_internal_key, new_key))
    new_unmasked_key = get_unmasked_key(new_key);

    get_creator_data(creator_data);
    check_internal_key(cur_unmasked_key, creator_data, new_unmasked_key);
    check_otbn_sideload(new_unmasked_key, "CreatorRootKey", cur_otbn_key);

    // OwnerIntKey
    `DV_WAIT(cfg.sw_logger_vif.printed_log == "Keymgr entered OwnerIntKey State");
    cur_unmasked_key = new_unmasked_key;
    `DV_CHECK_FATAL(uvm_hdl_read(path_internal_key, new_key))
    new_unmasked_key = get_unmasked_key(new_key);

    check_internal_key(cur_unmasked_key, get_owner_int_data(), new_unmasked_key);
    check_otbn_sideload(new_unmasked_key, "OwnerIntKey", new_otbn_key);
    `DV_CHECK_NE(cur_otbn_key, new_otbn_key);
    cur_otbn_key = new_otbn_key;

    // OwnerKey
    `DV_WAIT(cfg.sw_logger_vif.printed_log == "Keymgr entered OwnerKey State");
    cur_unmasked_key = new_unmasked_key;
    `DV_CHECK_FATAL(uvm_hdl_read(path_internal_key, new_key))
    new_unmasked_key = get_unmasked_key(new_key);

    check_internal_key(cur_unmasked_key, get_owner_root_data(), new_unmasked_key);
    check_otbn_sideload(new_unmasked_key, "OwnerKey", new_otbn_key);
    `DV_CHECK_NE(cur_otbn_key, new_otbn_key);

  endtask

endclass : chip_sw_keymgr_derive_sealing_vseq
