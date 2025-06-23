#include <gtest/gtest.h>
#include <types.h>

#include <string>
#include <vector>

TEST(TestObbDetection, Create) {
  const float ar[] = {1, 2, 3, 4, 6, 5};
  ASSERT_NO_THROW(ObbDetection::from_c_array(ar));
}

TEST(TestObbDetection, ToString) {
  const float ar[] = {1, 2, 3, 4, 6, 5};
  ObbDetection det = ObbDetection::from_c_array(ar);
  const std::string exp_string{"1.000 2.000 3.000 4.000 5 6.000"};
  ASSERT_EQ(det.to_string(), exp_string);
}

TEST(TestObbDetection, ToVec) {
  const float ar[] = {1, 2, 3, 4, 6, 5};
  const std::vector<float> vec{1, 2, 3, 4, 5, 6};
  ObbDetection det = ObbDetection::from_c_array(ar);
  ASSERT_EQ(det.to_vec(), vec);
}
