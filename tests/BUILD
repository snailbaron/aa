load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_test(
    name = "tests",
    size = "small",
    srcs = ["tests.cpp"],
    deps = [
        "//:aa",
        "//deps/catch",
    ],
)
