//
// Created by ivan on 28.04.2026.
//
#include <gtest/gtest.h>
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (!argv)
        ::testing::FLAGS_gtest_filter = "ConnectionHandlerTests.*";
    return RUN_ALL_TESTS();
}