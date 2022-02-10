cc_library(
    name = "soa",
    hdrs = glob(["vapid/*.h"]),
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "scratch",
    srcs = ["scratch.cc"],
    deps = [":soa"]
)

cc_binary(
    # deprecated in favor of benchmarks below
    name = "benchmark",
    srcs = ["benchmark.cc", "tictoc.hpp"],
    deps = [":soa"]
)

cc_binary(
    name = "benchmarks",
    srcs = ["benchmarks.cc"],
    deps = [":soa",
            "@com_github_google_benchmark//:benchmark_main"]
)
