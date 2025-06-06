#include "json_utils.h"
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;
void print_util() { std::cout << "Hello JSON world!"; }

cv::Mat parse_binary_image(const std::vector<uchar> data) {
  // assume the string is binary64 encoding of image

  // cv::Mat m = cv::Mat(cv::Size(810, 1080), CV_32FC3, data).clone();

  cv::Mat img = cv::imdecode(data, cv::IMREAD_COLOR_RGB).clone();
  return img;
}

void print_base_json(const json &js) {
  for (auto i : js) {
    std::cout << i << std::endl;
  }
  std::cout << "\n";
}

std::map<std::string, std::string> parse_object_msg(const json &js) {
  std::map<std::string, std::string> mp{};

  if (js.contains("__type__")) {
    mp["type"] = js["__type__"];
  }
  if (js.contains("__value__")) {
    mp["value"] = js["__value__"];
    std::vector<uchar> v(js["__value__"].begin(), js["__value__"].end());
    cv::Mat mat = parse_binary_image(js["__value__"]);
    std::cout << "CV Mat size: " << mat.size << std::endl;
    std::cout << "CV Mat : " << *mat.data << std::endl;
  }
  return mp;
}

std::string parse_string_msg(const json &js) {
  return js.template get<std::string>();
}

std::vector<std::string> parse_array_msg(const json &js,
                                         std::vector<std::string> &v) {
  for (auto i : js) {
    if (i.is_string()) {
      v.push_back(i);
    } else if (i.is_array()) {
      parse_array_msg(i, v);
    } else if (i.is_object()) {
      auto mp = parse_object_msg(i);
      v.push_back(mp["type"]);
      v.push_back(mp["value"]);
    }
  }
  return v;
}

WhichMsg determine_msg(const nlohmann::json &js) {
  if (js.is_string()) {
    return STRING_MSG;
  } else if (js.is_array()) {
    return ARRAY_MSG;
  } else if (js.is_object()) {
    return OBJECT_MSG;
  }
  return OBJECT_MSG;
}
