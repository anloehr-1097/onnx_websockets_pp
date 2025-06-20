#ifndef SRC_ONNXINFERLIB_TYPES
#define SRC_ONNXINFERLIB_TYPES

#include <string>
#include <vector>

struct ObbDetection {
private:
  float x1;
  float y1;
  float x2;
  float y2;
  int label;
  float confidence;
  ObbDetection(float x1, float y1, float x2, float y2, float confidence,
               int label);

public:
  static ObbDetection from_c_array(const float *ar) {
    return ObbDetection(*ar, *(ar + 1), *(ar + 2), *(ar + 3), *(ar + 4),
                        static_cast<int>(*(ar + 5)));
  }
  std::string to_string();
  std::vector<float> to_vec();
};

#endif // SRC_ONNXINFERLIB_TYPES
