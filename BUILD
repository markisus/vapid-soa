COPTS = select({
    "@bazel_tools//src/conditions:windows": ["/std:c++17"],
    "//conditions:default": ["--std=c++17"],
})    

cc_library(
    name = "soa",
    hdrs = glob(["vapid/*.h"]),
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "scratch",
    srcs = ["scratch.cc"],
    deps = [":soa"],
    copts=COPTS
)

cc_binary(
    # deprecated in favor of benchmarks below
    name = "benchmark",
    srcs = ["benchmark.cc", "tictoc.hpp"],
    deps = [":soa"],
    copts=COPTS
)

cc_binary(
    name = "benchmarks",
    srcs = ["benchmarks.cc"],
    deps = [":soa",
            "@com_github_google_benchmark//:benchmark_main"],
    copts=COPTS
)
