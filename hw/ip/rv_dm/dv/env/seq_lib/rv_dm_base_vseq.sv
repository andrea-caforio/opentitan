// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

class rv_dm_base_vseq extends cip_base_vseq #(
    .RAL_T               (rv_dm_regs_reg_block),
    .CFG_T               (rv_dm_env_cfg),
    .COV_T               (rv_dm_env_cov),
    .VIRTUAL_SEQUENCER_T (rv_dm_virtual_sequencer)
  );
  `uvm_object_utils(rv_dm_base_vseq)
  `uvm_object_new

  // These flags control "late debug enable". The mode (late_debug_enable) gets randomized in
  // pre_start and it takes effect either through a top-level pin (pin_late_debug_enable) or a
  // register (reg_late_debug_enable).
  //
  // When one of this inputs is mubi true, the "debug enable" check is made on lc_hw_debug_en_i
  // instead of lc_dft_en_i.
  rand bit late_debug_enable;
  rand bit pin_late_debug_enable;
  rand bit reg_late_debug_enable;

  // This flag controls whether the pinmux_hw_debug_en_i signal is set to On. This determines
  // whether the JTAG interface is connected.
  rand bit pinmux_hw_debug_en;

  // This flag controls whether the lc_hw_debug_clr_i signal is set to On.
  rand bit lc_hw_debug_clr;

  // This flag controls whether the lc_hw_debug_en_i signal is set to On. When late debug mode is
  // enabled (controlled by late_debug_enable), this controls whether debug is enabled.
  rand bit lc_hw_debug_en;

  // This flag controls whether the lc_dft_en_i signal is set to On. When late debug mode is
  // disabled (controlled by late_debug_enable), this controls whether debug is enabled.
  rand bit lc_dft_en;

  // This flag controls whether the scanmode_i signal is set to On, putting the JTAG TAP in the
  // debug module into testmode and controlling TCK and TRST_N with the system clock and reset,
  // instead of the signals in jtag_if
  rand bit scanmode;

  rand logic [NUM_HARTS-1:0]  unavailable;

  rand int unsigned tck_period_ps;
  constraint tck_period_ps_c {
    tck_period_ps dist {
      [100_000:200_000] :/ 1,  // 5-10MHz
      [200_001:420_000] :/ 1,  // 2.4-5MHz
      [420_001:1000_000] :/ 1  // 1-2.4MHz
    };
  }

  // A constraint that disables scanmode. This is generally needed because scan mode breaks JTAG
  // access for the usual jtag_driver: the driver sees a different clock from the TAP and everything
  // quickly gets out of sync.
  //
  // A vseq that actually wants to exercise scanmode should override this constraint and turn it
  // back on.
  constraint no_scanmode_c {
    scanmode == 1'b0;
  }

  // A constraint that ensures debug is enabled. We will have sequences that wish to disable debug,
  // but they can do so by either disabling it in the middle of the sequence or by overriding this
  // constraint.
  constraint debug_enabled_c {
    lc_hw_debug_clr == 1'b0;
    lc_hw_debug_en == 1'b1;
  }

  // A constraint that asserts pinmux_hw_debug_en_i will be On. Similarly to how it uses the
  // debug_enable_c constraint, rv_dm_base_vseq constrains pinmux_hw_debug_en so that JTAG is
  // connected. To see the disconnected case, a subclass can override this constraint.
  constraint pinmux_hw_debug_en_c {
    pinmux_hw_debug_en == 1'b1;
  }

  // TODO(#23096): Currently, the dft enable (used when late debug is enable is false) is hard-coded
  //               to match lc_hw_debug_en. This eventually needs to be separately controlled.
  constraint lc_dft_en_c {
    lc_dft_en == lc_hw_debug_en;
  }

  // TODO(#23096): We don't currently test the situation where late debug enable is false. We
  // should.
  constraint late_debug_enable_c {
    late_debug_enable == 1;
  }

  // If we are running without a scoreboard then avoid setting the late_debug_enable register to a
  // value other than the default (false). The reason is that we have standard CSR check sequences
  // that, when there is no scoreboard, assert the value matches the reset value. This doesn't work
  // if we call set_late_debug_enable_with_reg(1) from dut_init.
  constraint no_reg_late_debug_enable_when_no_scb_c {
    cfg.en_scb == 1'b0 -> reg_late_debug_enable == 1'b0;
  }

  // A constraint to make sure that pin_late_debug_enable and reg_late_debug_enable correctly
  // implement the intent in the late_debug_enable bit.
  constraint late_debug_enable_split_c {
    late_debug_enable == pin_late_debug_enable || reg_late_debug_enable;
  }

  // SBA TL device sequence. Class member for more controllability.
  protected cip_tl_device_seq m_tl_sba_device_seq;

  // Handles for convenience.
  jtag_dtm_reg_block jtag_dtm_ral;
  jtag_dmi_reg_block jtag_dmi_ral;
  rv_dm_mem_reg_block tl_mem_ral;
  dv_base_reg_block dv_base_ral;

  virtual function void set_handles();
    super.set_handles();
    jtag_dtm_ral = cfg.m_jtag_agent_cfg.jtag_dtm_ral;
    jtag_dmi_ral = cfg.jtag_dmi_ral;
    dv_base_ral = cfg.ral_models["rv_dm_mem_reg_block"];
    `downcast(tl_mem_ral,dv_base_ral);
  endfunction

  task pre_start();
    cfg.rv_dm_vif.scanmode <= bool_to_mubi4_t(scanmode);

    cfg.rv_dm_vif.unavailable <= unavailable;

    cfg.rv_dm_vif.lc_dft_en <= bool_to_lc_tx_t(lc_dft_en);

    cfg.rv_dm_vif.lc_check_byp_en <= lc_ctrl_pkg::Off;
    cfg.rv_dm_vif.lc_escalate_en <= lc_ctrl_pkg::Off;
    cfg.rv_dm_vif.strap_en_override <= 1'b0;
`ifdef USE_DMI_INTERFACE
    // TODO: revisit this. In order to operate in DMI mode we need to assert `strap_en`.
    cfg.rv_dm_vif.strap_en <= 1'b1;
`else
    cfg.rv_dm_vif.strap_en <= 1'b0;
`endif

    // Drive the otp_dis_rv_dm_late_debug_i pin to match pin_late_debug_enable (to avoid assertions
    // that get triggered in prim_lc_sync/prim_mubi8_sync if the input is 'x). We will configure the
    // register a little later, in dut_init.
    set_late_debug_enable_with_pin(pin_late_debug_enable);

    // Drive the pinmux_hw_debug_en_i pin to match the pinmux_hw_debug_en bit, avoiding assertions
    // that get triggered in prim_lc_sync if the input is 'x.
    upd_pinmux_hw_debug_en();

    // Drive the lc_hw_debug_en_i pin to match the lc_hw_debug_en bit, avoiding assertions that get
    // triggered in prim_lc_sync if the input is 'x.
    upd_lc_hw_debug_en();

    super.pre_start();
  endtask

  virtual task dut_init(string reset_kind = "HARD");
    super.dut_init();

    // If JTAG is connected, it might be that it was previously disconnected and we need to tell the
    // jtag driver (which monitors its internal state) to start again.
    //
    // Note that we also want to do this at the start of the simulation. If this doesn't happen, you
    // can end up in a situation where jtag_driver sees the posedge on trst_n (not yet connected)
    // and starts to run reset_internal_state. After a few cycles, the pinmux_hw_debug_en_i signal
    // makes it through u_pm_en_sync and the jtag interface gets connected to the DAP... for the
    // second half of the reset sequence.
    if (pinmux_hw_debug_en) begin
      cfg.m_jtag_agent_cfg.jtag_if_connected.trigger();
    end

    // Write the late debug enable register with the value that we chose in pre_start when
    // randomizing. If there is no scoreboard, check that reg_late_debug_enable is false (so we can
    // leave the register at its default value) and skip this step. See the note above
    // no_reg_late_debug_enable_when_no_scb_c for an explanation.
    if (cfg.en_scb == 1'b0) begin
      `DV_CHECK(!reg_late_debug_enable)
    end else begin
      set_late_debug_enable_with_reg(reg_late_debug_enable);
    end

    // TODO: Randomize the contents of the debug ROM & the program buffer once out of reset.
    if (pinmux_hw_debug_en) begin
      // We would like to do a DMI transaction here. If this vseq is the first with debug enabled,
      // the "enable" signal will need to make it through the a prim_lc_sync in the design before it
      // takes effect. Fortunately, we can see that this has happened by looking at the trst_n
      // signal: it will go high once everything has been connected. *That* signal is exposed
      // through jtag_mon_if in the tb, which is visible through the jtag agent's mon_vif interface.
      // Exit early if a system reset appears in the meantime.
`ifndef USE_DMI_INTERFACE
      fork begin : isolation_fork
        fork
          wait(cfg.m_jtag_agent_cfg.mon_vif.trst_n);
          wait(!cfg.clk_rst_vif.rst_n);
        join_any
        disable fork;
      end join
      if (!cfg.clk_rst_vif.rst_n) return;
`endif

      // "Activate" the DM to facilitate ease of testing, but only if not in scanmode (where the
      // JTAG driver won't work properly). This will exit early if there is a JTAG reset.
      if (!scanmode) begin
        csr_wr(.ptr(jtag_dmi_ral.dmcontrol.dmactive), .value(1), .blocking(1), .predict(1));
      end
    end
    // Start the SBA TL device seq.
    sba_tl_device_seq_start();
  endtask

  // Have scan reset also applied at the start.
  virtual task apply_reset(string kind = "HARD");
    cfg.m_jtag_agent_cfg.vif.set_tck_period_ps(tck_period_ps);
    fork
      if (kind inside {"HARD", "TRST"}) begin
        cfg.m_jtag_agent_cfg.vif.do_trst_n();
      end
      if (kind inside {"HARD", "SCAN"}) apply_scan_reset();
      cfg.clk_lc_rst_vif.apply_reset();
      super.apply_reset(kind);
    join
  endtask

  // Apply scan reset.
  virtual task apply_scan_reset();
    uint delay;
    `DV_CHECK_STD_RANDOMIZE_WITH_FATAL(delay, delay inside {[0:1000]};) // ns
    #(delay * 1ns);
    cfg.rv_dm_vif.cb.scan_rst_n <= 1'b0;
    // Wait for core clock cycles.
    `DV_CHECK_STD_RANDOMIZE_WITH_FATAL(delay, delay inside {[2:50]};) // cycles
    cfg.clk_rst_vif.wait_clks(delay);
    `DV_CHECK_STD_RANDOMIZE_WITH_FATAL(delay, delay inside {[0:1000]};) // ns
    cfg.rv_dm_vif.cb.scan_rst_n <= 1'b1;
  endtask

  virtual task apply_resets_concurrently(int reset_duration_ps = 0);
    int trst_n_duration_ps = cfg.m_jtag_agent_cfg.vif.tck_period_ps * $urandom_range(5, 20);
    cfg.rv_dm_vif.cb.scan_rst_n <= 1'b0;
    cfg.m_jtag_agent_cfg.vif.trst_n <= 1'b0;
    super.apply_resets_concurrently(dv_utils_pkg::max2(reset_duration_ps, trst_n_duration_ps));
    cfg.m_jtag_agent_cfg.vif.trst_n <= 1'b1;
    cfg.rv_dm_vif.cb.scan_rst_n <= 1'b1;
  endtask

  virtual task dut_shutdown();
    sba_tl_device_seq_stop();
  endtask

  // Spawns off a thread to auto-respond to incoming TL accesses on the SBA host interface.
  virtual task sba_tl_device_seq_start(int min_rsp_delay = 0,
                                       int max_rsp_delay = 80,
                                       int rsp_abort_pct = 25,
                                       int d_error_pct = 0,
                                       int d_chan_intg_err_pct = 0);
    m_tl_sba_device_seq = cip_tl_device_seq::type_id::create("m_tl_sba_device_seq");
    m_tl_sba_device_seq.min_rsp_delay = min_rsp_delay;
    m_tl_sba_device_seq.max_rsp_delay = max_rsp_delay;
    m_tl_sba_device_seq.rsp_abort_pct = rsp_abort_pct;
    m_tl_sba_device_seq.d_error_pct = d_error_pct;
    m_tl_sba_device_seq.d_chan_intg_err_pct = d_chan_intg_err_pct;
    `DV_CHECK_RANDOMIZE_FATAL(m_tl_sba_device_seq)
    `uvm_info(`gfn, "Started running m_tl_sba_device_seq", UVM_MEDIUM)
    fork m_tl_sba_device_seq.start(p_sequencer.tl_sba_sequencer_h); join_none
    // To ensure the seq above starts executing before the code following it starts executing.
    #0;
    // TODO: sba_tl_device_seq_disable_tlul_assert_host_sba_resp_svas();
  endtask

  // Stop running the m_tl_sba_device_seq seq.
  //
  // This is a no-op if the sequence is actually null (because we never completed dut_init, which
  // would have constructed the object),
  virtual task sba_tl_device_seq_stop();
    if (m_tl_sba_device_seq != null) begin
      m_tl_sba_device_seq.seq_stop();
      `uvm_info(`gfn, "Stopped running m_tl_sba_device_seq", UVM_MEDIUM)
    end
  endtask

  // Task forked off to disable TLUL host SBA assertions when injecting intg errors on the response
  // channel.
  virtual task sba_tl_device_seq_disable_tlul_assert_host_sba_resp_svas();
    fork
      begin: isolation_thread
        fork
          forever @m_tl_sba_device_seq.inject_d_chan_intg_err begin
            cfg.rv_dm_vif.disable_tlul_assert_host_sba_resp_svas =
                m_tl_sba_device_seq.inject_d_chan_intg_err;
          end
          m_tl_sba_device_seq.wait_for_sequence_state(UVM_FINISHED);
        join_any
        disable fork;
      end
    join_none
  endtask

  task read_dmcontrol(input bit backdoor, output dmcontrol_t value);
    uvm_reg_data_t raw;
    csr_rd(.ptr(jtag_dmi_ral.dmcontrol), .value(raw));
    value = dmcontrol_t'(raw);
  endtask

  // Tell rv_dm to request a halt, then "acknowledge" its forwarded request as the CPU after a few
  // cycles (hartsel=0 give a hart ID of 0 as we only have one hart).
  task request_halt();
    // The TLUL connection gets blocked by u_tlul_lc_gate_rom if there is an ndmreset signal. This
    // is confusing to debug, so use a backdoor read to check that it isn't currently set.
    dmcontrol_t dmcontrol_val;
    read_dmcontrol(.backdoor(1), .value(dmcontrol_val));
    if (!cfg.clk_rst_vif.rst_n) return;
    `DV_CHECK(!dmcontrol_val.ndmreset);

    csr_wr(.ptr(jtag_dmi_ral.dmcontrol.haltreq), .value(1));
    if (!cfg.clk_rst_vif.rst_n) return;
    `DV_CHECK_EQ(cfg.rv_dm_vif.cb.debug_req, 1)

    // Wait a short time (up to 10 cycles, but stopping early if there's a reset)
    fork begin : isolation_fork
      fork
        cfg.clk_rst_vif.wait_clks($urandom_range(0, 10));
        wait(!cfg.clk_rst_vif.rst_n);
      join_any
      disable fork;
    end join
    if (!cfg.clk_rst_vif.rst_n) return;

    csr_wr(.ptr(tl_mem_ral.halted), .value(0));
  endtask

  // Look to see whether we are currently in reset. If so, return immediately. If not, check whether
  // cfg.will_reset is set (showing that a higher-level sequence would like to inject a reset). If
  // so, allow the reset to be injected by pausing for a while without sending any CSR requests.
  //
  // In either reset situation, write 1 to the should_stop output argument so that a caller can
  // choose to stop afterwards.
  task spot_resets(output bit should_stop);
    should_stop = 1'b0;
    if (!cfg.clk_rst_vif.rst_n) begin
      should_stop = 1'b1;
      return;
    end
    if (cfg.stop_transaction_generators()) begin
      cfg.clk_rst_vif.wait_clks(CyclesWithNoAccessesThreshold * 2);
      should_stop = 1'b1;
    end
  endtask

  // Read the abstractcs register over DMI
  task read_abstractcs(output abstractcs_t value);
    uvm_reg_data_t raw;
    csr_rd(.ptr(jtag_dmi_ral.abstractcs), .value(raw));
    value = abstractcs_t'(raw);
  endtask

  function bit [31:0] bool_to_something(bit bool_val, int unsigned width, bit [31:0] true_val);
    bit [31:0] val;
    if (bool_val) begin
      val = true_val;
    end else begin
      `DV_CHECK_STD_RANDOMIZE_WITH_FATAL(val, val != true_val; val >> width == 32'h0;)
    end
    return val;
  endfunction

  function lc_ctrl_pkg::lc_tx_t bool_to_lc_tx_t(bit bool_val);
    return lc_ctrl_pkg::lc_tx_t'(bool_to_something(bool_val, 4, lc_ctrl_pkg::On));
  endfunction

  function prim_mubi_pkg::mubi4_t bool_to_mubi4_t(bit bool_val);
    return prim_mubi_pkg::mubi4_t'(bool_to_something(bool_val, 4, prim_mubi_pkg::MuBi4True));
  endfunction

  function prim_mubi_pkg::mubi8_t bool_to_mubi8_t(bit bool_val);
    return prim_mubi_pkg::mubi8_t'(bool_to_something(bool_val, 8, prim_mubi_pkg::MuBi8True));
  endfunction

  function prim_mubi_pkg::mubi32_t bool_to_mubi32_t(bit bool_val);
    return prim_mubi_pkg::mubi32_t'(bool_to_something(bool_val, 32, prim_mubi_pkg::MuBi32True));
  endfunction

  // Set the otp_dis_rv_dm_late_debug_i pin to a t/f value matching bool_val.
  function void set_late_debug_enable_with_pin(bit bool_val);
    cfg.rv_dm_vif.otp_dis_rv_dm_late_debug <= bool_to_mubi8_t(bool_val);
  endfunction

  // Write to the late_debug_enable register with a t/f value matching bool_val.
  virtual task set_late_debug_enable_with_reg(bit bool_val);
    csr_wr(.ptr(ral.late_debug_enable), .value(bool_to_mubi32_t(bool_val)));
  endtask

  // Update the pinmux_hw_debug_en_i pin to match the bit in pinmux_hw_debug_en
  function void upd_pinmux_hw_debug_en();
    cfg.rv_dm_vif.pinmux_hw_debug_en <= bool_to_lc_tx_t(pinmux_hw_debug_en);
  endfunction

  // Update the lc_hw_debug_clr_i and lc_hw_debug_en_i pins to match the bit in lc_hw_debug_clr and
  // lc_hw_debug_en, respectively.
  function void upd_lc_hw_debug_en();
    // The `bool_to_lc_tx_t` function converts a bit type to a lc_tx_t type by mapping `1` to `On`
    // and `0` to a random non-`On` value. For `lc_hw_debug_clr`, this is not what we want, though,
    // because the DUT will interpret a non-`Off` value as `On`. Hence the ternary statement below
    // instead maps `0` to `Off`.
    cfg.rv_dm_vif.lc_hw_debug_clr <= lc_hw_debug_clr ? lc_ctrl_pkg::On : lc_ctrl_pkg::Off;
    cfg.rv_dm_vif.lc_hw_debug_en <= bool_to_lc_tx_t(lc_hw_debug_en);
  endfunction

  // Read the dtmcs register and check the dmistat field has the expected value.
  task check_dmistat(bit [1:0] expected_dmistat);
    uvm_reg_data_t rdata;
    csr_rd(.ptr(jtag_dtm_ral.dtmcs), .value(rdata));
    `DV_CHECK_EQ(expected_dmistat, get_field_val(jtag_dtm_ral.dtmcs.dmistat, rdata))
  endtask

  // Check that the cmderr field in abstractcs is as expected, skipping the check if the system is
  // in reset.
  task check_cmderr(cmderr_e cmderr_exp);
    abstractcs_t abstractcs;
    read_abstractcs(abstractcs);
    if (cfg.clk_rst_vif.rst_n) `DV_CHECK_EQ(abstractcs.cmderr, cmderr_exp);
  endtask

  // Clear the cmderr field of abstractcs.
  task clear_cmderr();
    // To clear the field, we use uvm_reg_field.predict() to convince the model that it has an error
    // that needs clearing, then call uvm_reg_field.set() with 3'b111 to set desired value to zero
    // (passing 3'b111 rather than zero because the field is R/W1C), then finally call update.
    uvm_status_e status;

    `DV_CHECK_FATAL(jtag_dmi_ral.abstractcs.cmderr.predict(3'b111));
    jtag_dmi_ral.abstractcs.cmderr.set(3'b111);
    jtag_dmi_ral.abstractcs.update(.status(status));
    if (cfg.clk_rst_vif.rst_n) `DV_CHECK_EQ(status, UVM_IS_OK);
  endtask

  // Generate an abstract command that tries to read the specified register
  function command_t gen_read_register_cmd(bit [15:0] regno);
    command_t cmd;
    ac_ar_cmd_t ar_cmd = '0;

    ar_cmd.aarsize = 2; // Access lower 32 bits
    ar_cmd.aarpostincrement = 0;
    ar_cmd.postexec = 0;
    ar_cmd.transfer = 0;
    ar_cmd.write = 0;
    ar_cmd.regno = regno;

    cmd.cmdtype = AccessRegister;
    cmd.control = ar_cmd;
    return cmd;
  endfunction
endclass : rv_dm_base_vseq
