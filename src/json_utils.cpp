#include "json_utils.h"
#include "base64.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <string>
#include <variant>
#include <vector>

using json = nlohmann::json;
void print_util() { std::cout << "Hello JSON world!"; }

void write_base64_to_file(const std::string &data) {
  std::string fname("b64img.txt");
  std::ofstream fp(fname);
  fp << data;
  fp.close();
}
cv::Mat parse_binary_image(const std::vector<uchar> &data) {
  // assume the string is binary64 encoding of image
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << "decoded_img_"
     << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S")
     << ".jpeg";

  cv::Mat img = cv::imdecode(data, cv::IMREAD_UNCHANGED).clone();
  cv::imwrite(ss.str(), img);
  return img;
}

void print_base_json(const json &js) {
  for (auto i : js) {
    std::cout << i << std::endl;
  }
  std::cout << "\n";
}

std::string parse_string_msg(const json &js) {
  return js.template get<std::string>();
}

WhichMsg determine_msg_type(const nlohmann::json &js) {
  if (js.is_string()) {
    return STRING_MSG;
  } else if (js.is_array()) {
    return ARRAY_MSG;
  } else if (js.is_object()) {
    return OBJECT_MSG;
  }
  return OBJECT_MSG;
}

std::variant<cv::Mat, int> get_image(const nlohmann::json &js) {
  if (!js[0][0].contains("__value__")) {
    return -1;
  }
  std::string b64_str{js[0][0]["__value__"]};
  std::string b64dec(base64_decode(b64_str));
  std::vector<uchar> v(b64dec.begin(), b64dec.end());
  cv::Mat mat = parse_binary_image(v);
  return mat;
}

std::variant<std::string, int> get_hello_message(const nlohmann::json &js) {
  if (determine_msg_type(js) == STRING_MSG) {
    return parse_string_msg(js);
  } else if (determine_msg_type(js) == ARRAY_MSG) {

    for (auto elem : js) {

      auto ret = get_hello_message(elem);
      if (std::holds_alternative<std::string>(ret)) {
        return ret;
      };
    }
  } else {
    return -1;
  }
}

std::string find_in_json(const nlohmann::json &js, const std::string val) {
  if (js.contains(val)) {
    return js[val];
  }
  // not in js --> iterate through json
  if (determine_msg_type(js) == ARRAY_MSG) {
    for (auto elem : js) {
      auto res = find_in_json(elem, val);
      return res;
    }
  } else if (determine_msg_type(js) == OBJECT_MSG) {
  }
}

std::vector<nlohmann::json> parse_json_array(const nlohmann::json &js,
                                             std::vector<nlohmann::json> &v) {}

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

std::map<std::string, std::string> parse_object_msg(const json &js) {
  std::map<std::string, std::string> mp{};

  if (js.contains("__type__")) {
    mp["type"] = js["__type__"];
  }
  if (js.contains("__value__")) {
    mp["value"] = js["__value__"];
    write_base64_to_file(mp["value"]);
    std::string b64dec(base64_decode(mp["value"]));
    // std::vector<uchar> v(js["__value__"].begin(), js["__value__"].end());
    std::vector<uchar> v(b64dec.begin(), b64dec.end());
    // cv::Mat mat = parse_binary_image(js["__value__"]);
    cv::Mat mat = parse_binary_image(v);
    // cv::imwrite("decoded_img.png", mat);
    std::cout << "CV Mat size: " << mat.size << std::endl;
  }
  return mp;
}
