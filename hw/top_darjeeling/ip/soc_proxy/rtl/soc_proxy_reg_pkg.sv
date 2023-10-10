// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Register Package auto-generated by `reggen` containing data structure

package soc_proxy_reg_pkg;

  // Param list
  parameter int unsigned NumExternalIrqs = 32;
  parameter int NumAlerts = 29;

  // Address widths within the block
  parameter int CoreAw = 4;
  parameter int CtnAw = 1;

  ///////////////////////////////////////////////
  // Typedefs for registers for core interface //
  ///////////////////////////////////////////////

  typedef struct packed {
    logic [31:0] q;
  } soc_proxy_reg2hw_intr_state_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } soc_proxy_reg2hw_intr_enable_reg_t;

  typedef struct packed {
    logic [31:0] q;
    logic        qe;
  } soc_proxy_reg2hw_intr_test_reg_t;

  typedef struct packed {
    struct packed {
      logic        q;
      logic        qe;
    } fatal_alert_intg;
    struct packed {
      logic        q;
      logic        qe;
    } fatal_alert_external_0;
    struct packed {
      logic        q;
      logic        qe;
    } fatal_alert_external_1;
    struct packed {
      logic        q;
      logic        qe;
    } fatal_alert_external_2;
    struct packed {
      logic        q;
      logic        qe;
    } fatal_alert_external_3;
    struct packed {
      logic        q;
      logic        qe;
    } fatal_alert_external_4;
    struct packed {
      logic        q;
      logic        qe;
    } fatal_alert_external_5;
    struct packed {
      logic        q;
      logic        qe;
    } fatal_alert_external_6;
    struct packed {
      logic        q;
      logic        qe;
    } fatal_alert_external_7;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_0;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_1;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_2;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_3;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_4;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_5;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_6;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_7;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_8;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_9;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_10;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_11;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_12;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_13;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_14;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_15;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_16;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_17;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_18;
    struct packed {
      logic        q;
      logic        qe;
    } recov_alert_external_19;
  } soc_proxy_reg2hw_alert_test_reg_t;

  typedef struct packed {
    logic [31:0] d;
    logic        de;
  } soc_proxy_hw2reg_intr_state_reg_t;

  // Register -> HW type for core interface
  typedef struct packed {
    soc_proxy_reg2hw_intr_state_reg_t intr_state; // [154:123]
    soc_proxy_reg2hw_intr_enable_reg_t intr_enable; // [122:91]
    soc_proxy_reg2hw_intr_test_reg_t intr_test; // [90:58]
    soc_proxy_reg2hw_alert_test_reg_t alert_test; // [57:0]
  } soc_proxy_core_reg2hw_t;

  // HW -> register type for core interface
  typedef struct packed {
    soc_proxy_hw2reg_intr_state_reg_t intr_state; // [32:0]
  } soc_proxy_core_hw2reg_t;

  // Register offsets for core interface
  parameter logic [CoreAw-1:0] SOC_PROXY_INTR_STATE_OFFSET = 4'h 0;
  parameter logic [CoreAw-1:0] SOC_PROXY_INTR_ENABLE_OFFSET = 4'h 4;
  parameter logic [CoreAw-1:0] SOC_PROXY_INTR_TEST_OFFSET = 4'h 8;
  parameter logic [CoreAw-1:0] SOC_PROXY_ALERT_TEST_OFFSET = 4'h c;

  // Reset values for hwext registers and their fields for core interface
  parameter logic [31:0] SOC_PROXY_INTR_TEST_RESVAL = 32'h 0;
  parameter logic [31:0] SOC_PROXY_INTR_TEST_EXTERNAL_RESVAL = 32'h 0;
  parameter logic [28:0] SOC_PROXY_ALERT_TEST_RESVAL = 29'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_FATAL_ALERT_INTG_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_FATAL_ALERT_EXTERNAL_0_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_FATAL_ALERT_EXTERNAL_1_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_FATAL_ALERT_EXTERNAL_2_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_FATAL_ALERT_EXTERNAL_3_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_FATAL_ALERT_EXTERNAL_4_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_FATAL_ALERT_EXTERNAL_5_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_FATAL_ALERT_EXTERNAL_6_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_FATAL_ALERT_EXTERNAL_7_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_0_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_1_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_2_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_3_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_4_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_5_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_6_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_7_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_8_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_9_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_10_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_11_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_12_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_13_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_14_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_15_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_16_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_17_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_18_RESVAL = 1'h 0;
  parameter logic [0:0] SOC_PROXY_ALERT_TEST_RECOV_ALERT_EXTERNAL_19_RESVAL = 1'h 0;

  // Register index for core interface
  typedef enum int {
    SOC_PROXY_INTR_STATE,
    SOC_PROXY_INTR_ENABLE,
    SOC_PROXY_INTR_TEST,
    SOC_PROXY_ALERT_TEST
  } soc_proxy_core_id_e;

  // Register width information to check illegal writes for core interface
  parameter logic [3:0] SOC_PROXY_CORE_PERMIT [4] = '{
    4'b 1111, // index[0] SOC_PROXY_INTR_STATE
    4'b 1111, // index[1] SOC_PROXY_INTR_ENABLE
    4'b 1111, // index[2] SOC_PROXY_INTR_TEST
    4'b 1111  // index[3] SOC_PROXY_ALERT_TEST
  };

endpackage
