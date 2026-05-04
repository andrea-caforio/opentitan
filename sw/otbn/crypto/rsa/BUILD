# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

# load("//rules:otbn.bzl", "otbn_library", "otbn_sim_test", "otbn_sim_test_suite")
# load("@rules_python//python:defs.bzl", "py_binary")
# load("//rules:otbn.bzl", "otbn_autogen_sim_test")
# load("@ot_python_deps//:requirements.bzl", "requirement")

load("//rules:otbn.bzl", "otbn_binary", "otbn_library")

package(default_visibility = ["//visibility:public"])

otbn_library(
    name = "mont",
    srcs = [
        "mont.s",
    ],
)

otbn_library(
    name = "rsa_modexp",
    srcs = [
        "rsa_modexp.s",
    ],
)

otbn_library(
    name = "rsa_inv",
    srcs = [
        "rsa_inv.s",
    ],
)

otbn_library(
    name = "rsa_gcd",
    srcs = [
        "rsa_gcd.s",
    ],
)

otbn_library(
    name = "rsa_utils",
    srcs = [
        "rsa_utils.s",
    ],
)

# otbn_sim_test(
#     name = "mont_test",
#     srcs = [
#         "mont.s",
#         "//sw/otbn/crypto:montmul.s",
#     ],
#     testcase = "mont_test.hjson",
# )

# py_binary(
#     name = "mont_testgen",
#     srcs = ["mont_testgen.py"],
#     imports = [
#         "../../../../hw/ip/otbn/util",
#     ],
#     deps = [
#         "//hw/ip/otbn/util/shared:testgen",
#     ],
# )

# # Run randomly generated tests. Note that the random test data will be the same
# # for the same seed every time, because Bazel always builds "hermetically" and
# # therefore cannot generate any actually random values at build time.
# [
#     otbn_autogen_sim_test(
#         name = "mont_test" + str(i),
#         srcs = [
#             "mont.s",
#             "//sw/otbn/crypto:montmul.s",
#         ],
#         seed = i,
#         testgen = ":mont_testgen",
#         type = "testcase",
#     )
#     for i in range(1)
# ]
