#include <string>
#include <gtest/gtest.h>
#include "inference.h"

TEST(CustFunc, Test1) {
    // test definition, see offiicial Docu
    std::string s = "Hello from test_cust_func!";
    ASSERT_EQ(s, s);
}
 
int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);  // initialize GTest
    return RUN_ALL_TESTS();
}
