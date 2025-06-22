#include <opencv2/opencv.hpp>
#include <string>

void show_image(const std::string fpath);
void normalize_img(cv::Mat &img, cv::Mat &result);
cv::Mat preprocess_img(cv::Mat &src_im, cv::Size sz, bool normalize);
