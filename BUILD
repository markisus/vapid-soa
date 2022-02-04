cc_library(
    name = "soa",
    hdrs = glob(["vapid/*.h"]),
)

cc_binary(
    name = "scratch",
    srcs = ["scratch.cc"],
    deps = [":soa"]
)
