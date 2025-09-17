# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load(
    "//rules:cross_platform.bzl",
    "dual_cc_device_library_of",
    "dual_cc_library",
    "dual_inputs",
)
load(
    "//rules/opentitan:defs.bzl",
    "EARLGREY_TEST_ENVS",
    "fpga_params",
    "opentitan_test",
    "qemu_params",
    "verilator_params",
)
load(
    "@bazel_skylib//lib:dicts.bzl",
    "dicts",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "ecdsa_p256_key",
    srcs = ["ecdsa_p256_key.c"],
    hdrs = ["ecdsa_p256_key.h"],
    deps = [
        "//sw/device/lib/base:macros",
    ],
)

cc_library(
    name = "spx_key",
    srcs = ["spx_key.c"],
    hdrs = ["spx_key.h"],
    deps = [
        "//sw/device/lib/base:macros",
        "//sw/device/silicon_creator/lib/sigverify/sphincsplus:params",
    ],
)

cc_library(
    name = "ecdsa_p256_verify",
    srcs = ["ecdsa_p256_verify.c"],
    hdrs = ["ecdsa_p256_verify.h"],
    deps = [
        ":ecdsa_p256_key",
        "//sw/device/lib/base:hardened",
        "//sw/device/silicon_creator/lib:otbn_boot_services",
        "//sw/device/silicon_creator/lib/drivers:hmac",
    ],
)

opentitan_test(
    name = "ecdsa_p256_verify_functest",
    srcs = ["ecdsa_p256_verify_functest.c"],
    exec_env = dicts.add(
        EARLGREY_TEST_ENVS,
        {
            "//hw/top_earlgrey:fpga_cw310_test_rom": None,
            "//hw/top_earlgrey:fpga_cw340_test_rom": None,
        },
    ),
    deps = [
        ":ecdsa_p256_verify",
        "//sw/device/lib/testing/test_framework:ottf_main",
    ],
)

cc_library(
    name = "spx_verify",
    srcs = ["spx_verify.c"],
    hdrs = ["spx_verify.h"],
    deps = [
        ":spx_key",
        "//hw/top:dt",
        "//hw/top:otp_ctrl_c_regs",
        "//sw/device/lib/base:hardened",
        "//sw/device/silicon_creator/lib/drivers:hmac",
        "//sw/device/silicon_creator/lib/drivers:lifecycle",
        "//sw/device/silicon_creator/lib/drivers:otp",
        "//sw/device/silicon_creator/lib/sigverify/sphincsplus:verify",
    ],
)

opentitan_test(
    name = "spx_verify_functest",
    srcs = ["spx_verify_functest.c"],
    exec_env = EARLGREY_TEST_ENVS,
    verilator = verilator_params(
        timeout = "long",
        tags = [
            "manual",
        ],
    ),
    deps = [
        ":spx_verify",
        "//sw/device/lib/testing/test_framework:ottf_main",
    ],
)

cc_library(
    name = "usage_constraints",
    srcs = ["usage_constraints.c"],
    hdrs = ["usage_constraints.h"],
    deps = [
        "//hw/top:dt",
        "//hw/top:otp_ctrl_c_regs",
        "//sw/device/lib/base:bitfield",
        "//sw/device/lib/base:hardened",
        "//sw/device/silicon_creator/lib:manifest",
        "//sw/device/silicon_creator/lib/drivers:otp",
    ],
)

cc_library(
    name = "flash_exec",
    hdrs = ["flash_exec.h"],
    deps = [
        ":spx_verify",
    ],
)

cc_test(
    name = "flash_exec_unittest",
    srcs = ["flash_exec_unittest.cc"],
    deps = [
        ":flash_exec",
        "//hw/top:flash_ctrl_c_regs",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "sigverify",
    hdrs = ["sigverify.h"],
    deps = [
        ":ecdsa_p256_verify",
        ":spx_verify",
        ":usage_constraints",
    ],
)
