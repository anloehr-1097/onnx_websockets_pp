#include "utils.h"
#define DEBUG 1

void print_image(const std::string fpath) {
  cv::Mat image = cv::imread(fpath, cv::IMREAD_COLOR_RGB);

  std::cout << image.size();
  std::cout << image.channels();
  std::cout << image.type();
  std::cout << std::endl;

  if (!image.data) {
    printf("No image data \n");
  } else {
    cv::namedWindow("Display Image", cv::WINDOW_NORMAL);
    cv::imshow("Display Image", image);
    cv::waitKey(0);
  };
};

cv::Mat im_normalize(cv::Mat img) {

  // image net normalization params
  cv::Scalar mean{0.485, 0.456, 0.406};
  cv::Scalar stddev{0.229, 0.224, 0.225};
  cv::Mat normalized;
  cv::subtract(img, mean, normalized);
  cv::divide(normalized, stddev, normalized);
  return normalized;
}

void normalize_img(cv::Mat &img, cv::Mat &result) {
  /*
   * normalize with mean and stddev
   * */

  // image net normalization params
  cv::Scalar mean{0.485, 0.456, 0.406};
  cv::Scalar stddev{0.229, 0.224, 0.225};
  std::vector<cv::Mat> channels(3);
  cv::split(img, channels);
  for (int c = 0; c < 3; c++) {
    channels[c] = (channels[c] - mean[c]) / stddev[c];
  }
  // combine 3 channels and write to result
  cv::merge(channels, result);

  // for illustration purposes
  cv::Mat save_normal;
  result.convertTo(save_normal, CV_8UC3, 255.0);
  cv::imwrite("Normalized.jpeg", save_normal);
};

cv::Mat preprocess_img(cv::Mat &src_im, cv::Size sz = cv::Size(224, 224),
                       bool normalize = 0) {
  /*
   * resize image with cubic interpolation, then optinally normalize
   */

  // resize
  cv::Mat resized_img;
  cv::resize(src_im, resized_img, sz, cv::INTER_CUBIC);
  cv::Mat final;

  cv::Mat tmp_img;
  resized_img.convertTo(tmp_img, CV_32FC3, 1.0 / 255.0);

  return tmp_img;
  // if (normalize) {
  //   // normalize_img(tmp_img, final);
  //   final = im_normalize(tmp_img);
  //   cv::Mat final_save;
  //   cv::Mat final_return;
  //   // final.convertTo(final_save, CV_8UC3, 255.0);
  //   final_return = final * 255.0;
  //   final_return.convertTo(final_save, CV_8UC3);
  //
  //   if (DEBUG) {
  //     cv::imwrite("after_resize.jpeg", resized_img);
  //     cv::imwrite("after_preprocess.jpeg", final_save);
  //   }
  //   // return final;
  //   return final_return;
  // } else return tmp_img;
};
