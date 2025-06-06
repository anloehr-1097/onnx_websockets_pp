#ifndef SRC_JSON_UTIL_H
#define SRC_JSON_UTIL_H
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

template <typename T> struct JM {
  T member;
  T get() { return member; }
  explicit JM(T _mem) : member(_mem) {}
};

enum WhichMsg { ARRAY_MSG, OBJECT_MSG, STRING_MSG };
void print_util();
void print_base_json(const nlohmann::json &);
void parse_hello_json_msg(const nlohmann::json &);
WhichMsg determine_msg(const nlohmann::json &);
std::string parse_string_msg(const nlohmann::json &);
std::vector<std::string> parse_array_msg(const nlohmann::json &,
                                         std::vector<std::string> &);
std::map<std::string, std::string> parse_object_msg(const nlohmann::json &);
void write_base64_to_file(const std::string &);

#endif // SRC_JSON_UTIL_H
