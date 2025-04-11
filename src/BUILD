load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "aa",
    srcs = [
        "error.cpp",
        "include/aa/error.hpp",
        "include/aa/options.hpp",
        "include/aa/parser.hpp",
        "parser.cpp",
    ],
    hdrs = [
        "include/aa.hpp",
    ],
    includes = ["include"],
    linkopts = [
        "-lstdc++exp",
    ],
    visibility = ["//visibility:public"],
)
