# BUILD file for use with https://github.com/dejwk/roo_testing.

cc_library(
    name = "roo_temperature",
    visibility = ["//visibility:public"],
    srcs = glob(
        [
            "src/**/*.cpp",
            "src/**/*.h",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    deps = [
        "//lib/roo_flags",
        "//lib/roo_logging",
        "//lib/roo_time"
    ],
)
