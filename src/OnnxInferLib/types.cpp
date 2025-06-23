#include "types.h"

#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <vector>

ObbDetection::ObbDetection(float x1, float y1, float x2, float y2,
                           float confidence, int label)
    : x1(x1), y1(y1), x2(x2), y2(y2), label(label), confidence(confidence) {}

std::string ObbDetection::to_string() {
  std::ostringstream ss;
  ss << std::fixed << std::setprecision(3) << x1 << " ";
  ss << std::fixed << std::setprecision(3) << y1 << " ";
  ss << std::fixed << std::setprecision(3) << x2 << " ";
  ss << std::fixed << std::setprecision(3) << y2 << " ";
  ss << std::fixed << std::setprecision(3) << label << " ";
  ss << std::fixed << std::setprecision(3) << confidence;
  return ss.str();
}

std::vector<float> ObbDetection::to_vec() {
  std::vector<float> vec;
  vec.reserve(6);
  vec.emplace_back(x1);
  vec.emplace_back(y1);
  vec.emplace_back(x2);
  vec.emplace_back(y2);
  vec.emplace_back(label);
  vec.emplace_back(confidence);
  return vec;
}
