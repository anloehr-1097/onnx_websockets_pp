#include<filesystem>
#include <gtest/gtest.h>
#include "inference.h"
#include "utils.h"



TEST(YOLOv11, TestYOLOv11SessionRun){

    auto fp = std::filesystem::path {"/Users/anlhr/Projects/onnx_websockets/models/yolo11x_obb.onnx"};
    auto onnx_sess = Yolov11Session(fp);
    onnx_sess.get_input_output_names();
    onnx_sess.get_output_type_info();
    // read image from disk & preprocess image
    // auto img = cv::imread("/Users/anlhr/Projects/onnx_websockets/images/test_img_broccoli.jpg", cv::IMREAD_COLOR_RGB);
    auto img = cv::imread("/Users/anlhr/Projects/onnx_websockets/images/test_img_broccoli.jpg", cv::IMREAD_COLOR_RGB);
    cv::Mat resized_img;
    img = preprocess_img(img, cv::Size(640, 640), 0);
    assert(img.type() == CV_32FC3 && "Image type must be CV_32FC3");
    assert(img.isContinuous() && "Image must be in contiguous memory");
    onnx_sess.set_input_image(img);
    auto onnx_out_tens = onnx_sess.detect();
    auto res = onnx_sess.postprocess(onnx_out_tens);
    ASSERT_EQ(res, 50);
}

