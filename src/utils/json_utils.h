#ifndef SRC_JSON_UTIL_H
#define SRC_JSON_UTIL_H
#include <map>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

template <typename T>
struct JM {
  T member;
  T get() { return member; }
  explicit JM(T _mem) : member(_mem) {}
};

enum WhichMsg { ARRAY_MSG, OBJECT_MSG, STRING_MSG };
void parse_hello_json_msg(const nlohmann::json &);
WhichMsg determine_msg_type(const nlohmann::json &);
std::string parse_string_msg(const nlohmann::json &);
std::vector<std::string> parse_array_msg(const nlohmann::json &,
                                         std::vector<std::string> &);
std::map<std::string, std::string> parse_object_msg(const nlohmann::json &);
void write_base64_to_file(const std::string &);

std::variant<cv::Mat, int> get_image(const nlohmann::json &);
std::variant<std::string, int> get_hello_message(const nlohmann::json &);

nlohmann::json to_js_string(const std::string_view &);

nlohmann::json write_celery_result_to_redis(const std::string &task_id,
                                            const std::string &result);

void save_json(nlohmann::json json, std::string fname);
#endif  // SRC_JSON_UTIL_H
