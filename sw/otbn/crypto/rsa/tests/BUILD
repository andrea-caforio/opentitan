# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load("//rules:otbn.bzl", "otbn_sim_test", "otbn_sim_testgen")
load("@rules_python//python:defs.bzl", "py_binary")

py_binary(
    name = "rsa_montmul_testgen",
    srcs = ["rsa_montmul_testgen.py"],
    imports = [
        "../../../../../hw/ip/otbn/util",
    ],
    deps = [
        "//hw/ip/otbn/util/shared:testgen",
    ],
)

otbn_sim_testgen(
    name = "rsa_montmul_rand_test",
    srcs = [
        "rsa_montmul_test.s",
        "//sw/otbn/crypto/rsa:mont.s",
    ],
    number = 50,
    testgen = ":rsa_montmul_testgen",
    type = "testcase",
)

py_binary(
    name = "rsa_compute_r2_testgen",
    srcs = ["rsa_utils.py"] + ["rsa_compute_r2_testgen.py"],
    imports = [
        "../../../../../hw/ip/otbn/util",
    ],
    deps = [
        "//hw/ip/otbn/util/shared:testgen",
    ],
)

otbn_sim_testgen(
    name = "rsa_compute_r2_rand_test",
    srcs = [
        "rsa_compute_r2_test.s",
        "//sw/otbn/crypto/rsa:mont.s",
        "//sw/otbn/crypto/rsa:rsa_gcd.s",
        "//sw/otbn/crypto/rsa:rsa_modexp.s",
    ],
    number = 10,
    testgen = ":rsa_compute_r2_testgen",
    type = "testcase",
)

py_binary(
    name = "rsa_compute_mu_testgen",
    srcs = ["rsa_compute_mu_testgen.py"],
    imports = [
        "../../../../../hw/ip/otbn/util",
    ],
    deps = [
        "//hw/ip/otbn/util/shared:testgen",
    ],
)

otbn_sim_testgen(
    name = "rsa_compute_mu_rand_test",
    srcs = [
        "rsa_compute_mu_test.s",
        "//sw/otbn/crypto/rsa:rsa_inv.s",
    ],
    number = 10,
    testgen = ":rsa_compute_mu_testgen",
    type = "testcase",
)

py_binary(
    name = "rsa_modexp_65537_testgen",
    srcs = ["rsa_modexp_65537_testgen.py"],
    imports = [
        "../../../../../hw/ip/otbn/util",
    ],
    deps = [
        "//hw/ip/otbn/util/shared:testgen",
    ],
)

otbn_sim_testgen(
    name = "rsa_modexp_65537_rand_test",
    srcs = [
        "rsa_modexp_65537_test.s",
        "//sw/otbn/crypto/rsa:mont.s",
    ],
    number = 100,
    testgen = ":rsa_modexp_65537_testgen",
    type = "testcase",
)

py_binary(
    name = "rsa_mod_f4_testgen",
    srcs = ["rsa_mod_f4_testgen.py"],
    imports = [
        "../../../../../hw/ip/otbn/util",
    ],
    deps = [
        "//hw/ip/otbn/util/shared:testgen",
    ],
)

otbn_sim_testgen(
    name = "rsa_mod_f4_rand_test",
    srcs = [
        "rsa_mod_f4_test.s",
        "//sw/otbn/crypto/rsa:rsa_gcd.s",
    ],
    number = 100,
    testgen = ":rsa_mod_f4_testgen",
    type = "testcase",
)

py_binary(
    name = "rsa_gcd_testgen",
    srcs = ["rsa_gcd_testgen.py"],
    imports = [
        "../../../../../hw/ip/otbn/util",
    ],
    deps = [
        "//hw/ip/otbn/util/shared:testgen",
    ],
)

otbn_sim_testgen(
    name = "rsa_gcd_rand_test",
    srcs = [
        "rsa_gcd_test.s",
        "//sw/otbn/crypto/rsa:rsa_gcd.s",
    ],
    number = 10,
    testgen = ":rsa_gcd_testgen",
    type = "testcase",
)

py_binary(
    name = "rsa_modinv_f4_testgen",
    srcs = ["rsa_modinv_f4_testgen.py"],
    imports = [
        "../../../../../hw/ip/otbn/util",
    ],
    deps = [
        "//hw/ip/otbn/util/shared:testgen",
    ],
)

otbn_sim_testgen(
    name = "rsa_modinv_f4_rand_test",
    srcs = [
        "rsa_modinv_f4_test.s",
        "//sw/otbn/crypto/rsa:mont.s",
        "//sw/otbn/crypto/rsa:rsa_gcd.s",
    ],
    number = 10,
    testgen = ":rsa_modinv_f4_testgen",
    type = "testcase",
)

py_binary(
    name = "rsa_blinding_testgen",
    srcs = ["rsa_blinding_testgen.py"],
    imports = [
        "../../../../../hw/ip/otbn/util",
    ],
    deps = [
        "//hw/ip/otbn/util/shared:testgen",
    ],
)

otbn_sim_testgen(
    name = "rsa_blinding_rand_test",
    srcs = [
        "rsa_blinding_test.s",
        "//sw/otbn/crypto/rsa:mont.s",
        "//sw/otbn/crypto/rsa:rsa_gcd.s",
        "//sw/otbn/crypto/rsa:rsa_modexp.s",
    ],
    number = 10,
    testgen = ":rsa_blinding_testgen",
    type = "testcase",
)

py_binary(
    name = "rsa_utils_testgen",
    srcs = ["rsa_utils_testgen.py"],
    imports = [
        "../../../../../hw/ip/otbn/util",
    ],
    deps = [
        "//hw/ip/otbn/util/shared:testgen",
    ],
)

otbn_sim_testgen(
    name = "rsa_utils_rand_test",
    srcs = [
        "rsa_utils_test.s",
        "//sw/otbn/crypto/rsa:rsa_utils.s",
    ],
    number = 100,
    testgen = ":rsa_utils_testgen",
    type = "testcase",
)

otbn_sim_test(
    name = "debug_test",
    srcs = [
        "rsa_mod_f4_test.s",
        "//sw/otbn/crypto/rsa:rsa_gcd.s",
    ],
    testcase = "debug_test.hjson",
)
