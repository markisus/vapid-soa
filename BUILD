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
