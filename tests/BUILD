load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_test(
    name = "tests",
    size = "small",
    srcs = ["tests.cpp"],
    deps = [
        "//:aa",
        "@catch2",
        "@catch2//:catch2_main",
    ],
)
