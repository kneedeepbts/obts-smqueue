#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <enet/enet.h>
#include <spdlog/spdlog.h>

int main(int argc, char** argv) {
    doctest::Context context;

//    context.addFilter("test-case-exclude", "*math*"); // exclude test cases with "math" in their name
//    context.setOption("abort-after", 5);              // stop test execution after 5 failed assertions
//    context.setOption("order-by", "name");            // sort the test cases by their name

    context.applyCommandLine(argc, argv);

    if(enet_initialize() != 0) {
        SPDLOG_ERROR("Failed to initialize UDP networking library.");
        return -1;
    }

    int res = context.run();

    atexit(enet_deinitialize);

    if(context.shouldExit()) {
        return res;
    }

    int client_stuff_return_code = 0;
    return res + client_stuff_return_code;
}
