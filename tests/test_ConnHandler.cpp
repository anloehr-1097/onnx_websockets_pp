#include "test_ConnHandlerFixture.h"
#include <gtest/gtest.h>

TEST(MyConnectionHandler, TestMyConnectionHandlerCreation) {}

TEST_F(ConnHandlerFixture, TestMyConnectionHandler) {
  EXPECT_TRUE(ba_available);
}

TEST_F(ConnHandlerFixture, TestMyConnectionHandlerChannelRegistration) {
  EXPECT_NO_THROW(handler.register_channel_callbacks());
}
