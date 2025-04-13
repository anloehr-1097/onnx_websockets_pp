#include <opencv2/opencv.hpp>
#include <string>

cv::Mat preprocess_img(cv::Mat& mat, cv::Size sz, bool);
void print_image(const std::string fpath);
cv::Mat img_normalize(cv::Mat img);
void normalize_img(cv::Mat &img, cv::Mat &result);
cv::Mat preprocess_img(cv::Mat& src_im, cv::Size sz, bool normalize);
