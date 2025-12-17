#include <gtest/gtest.h>
#include "../server/include/server.hpp"



TEST(ServerTest, Constructor) {
    Server server("127.0.0.1", 8888);
    EXPECT_EQ(server.getAddress(), "127.0.0.1");
    EXPECT_EQ(server.getPort(), 8888);
}
