#include "websocketpp/common/connection_hdl.hpp"
#include "websocketpp/frame.hpp"
#include <cstddef>
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <functional>
// #include <onnxruntime_cxx_api.h>
#include "inference.hpp"


#define DEBUG 1

typedef websocketpp::server<websocketpp::config::asio> server;
class utility_server;
void outside_handler(utility_server &us, websocketpp::connection_hdl hdl, server::message_ptr msg);
cv::Mat preprocess_img(cv::Mat& mat, cv::Size sz, bool);

class utility_server {
public:
    utility_server() {
         // Set logging settings
        m_endpoint.set_error_channels(websocketpp::log::elevel::all);
        m_endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
 
        // Initialize Asio
        m_endpoint.init_asio();

        /*
         * Need the std::bind here since we pass a member function  and the 'this' argument is passed
         * std::placeholders::_1 is corresponds to first arg passed call, analogously for 2
         * */

        // m_endpoint.set_message_handler(std::bind(
        //     &utility_server::duplicate_handler, this,
        //     std::placeholders::_1, std::placeholders::_2
        // ));
        //

        // m_endpoint.set_message_handler(std::function(&outside_handler));
        // m_endpoint.set_message_handler(std::bind(
        //     &outside_handler, this,
        //     std::placeholders::_1, std::placeholders::_2
        // ));

    };

    void send(websocketpp::connection_hdl hdl, std::string &s, websocketpp::frame::opcode::value op) {
        m_endpoint.send(hdl, s, op);
    };

    void set_message_handler(std::function<void(websocketpp::connection_hdl, server::message_ptr msg)> func){
        m_endpoint.set_message_handler(func);
    };
 
    void run() {
        // Listen on port 9002
        m_endpoint.listen(9002);
 
        // Queues a connection accept operation
        m_endpoint.start_accept();
 
        // Start the Asio io_service run loop
        m_endpoint.run();
    }

    void image_handler(websocketpp::connection_hdl hdl, server::message_ptr msg) {
        // verify that message type is binary
        m_endpoint.send(hdl, msg->get_payload(), msg->get_opcode());
    }

    void duplicate_handler(websocketpp::connection_hdl hdl, server::message_ptr msg) {
        // example handler member function impl
        // write a new message
        // do something with payload - here duplication
        std::string new_msg_payload(msg->get_raw_payload() + msg->get_raw_payload());  // non const ref to payload
        m_endpoint.send(hdl, new_msg_payload, msg->get_opcode());
    }

private:
    server m_endpoint;
};

// message handler used for websocket server
void outside_handler(utility_server &us, ResNetSession &sess, websocketpp::connection_hdl hdl, server::message_ptr msg) {
    // write a new message
    std::string new_msg_payload;
    if (msg->get_opcode() == websocketpp::frame::opcode::binary){
        std::vector<uchar> payload_2(msg->get_payload().begin(), msg->get_payload().end());
        cv::Mat img = cv::imdecode(payload_2, cv::IMREAD_COLOR_RGB);
        cv::Mat new_img = preprocess_img(img, cv::Size(224,224), 1);
        if (new_img.empty()) {
            throw std::runtime_error("Failed to decode image from byte string");
        }
        auto res = sess.detect(new_img);
        std::cout << "Result: " << res << std::endl;
        new_msg_payload = "Bytes frame. Image has size (" +
            std::to_string(new_img.rows) +
            "," + 
            std::to_string(new_img.cols) +
            ") yielding result: " +
            std::to_string(res); // non const ref to payload
    } 
    else {
        new_msg_payload = "This is a text frame"; // non const ref to payload
    };
    us.send(hdl, new_msg_payload, websocketpp::frame::opcode::text);
}


int print_image(const std::string fpath){
    cv::Mat image = cv::imread(fpath, cv::IMREAD_COLOR_RGB);

    std::cout << image.size();
    std::cout << image.channels();
    std::cout << image.type();
    std::cout << std::endl;

    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    cv::namedWindow("Display Image", cv::WINDOW_NORMAL);
    cv::imshow("Display Image", image);
    cv::waitKey(0);
 
    return 0;
};



cv::Mat im_normalize(cv::Mat img){

    // image net normalization params
    cv::Scalar mean {0.485, 0.456, 0.406};
    cv::Scalar stddev {0.229, 0.224, 0.225};
    cv::Mat normalized;
    cv::subtract(img, mean, normalized);
    cv::divide(normalized, stddev, normalized);
    return normalized;
}

void normalize_img(cv::Mat &img, cv::Mat &result){
    /* 
     * normalize with mean and stddev
     * */

    // image net normalization params
    cv::Scalar mean {0.485, 0.456, 0.406};
    cv::Scalar stddev {0.229, 0.224, 0.225};
    std::vector<cv::Mat> channels(3);
    cv::split(img, channels);
    for (int c = 0; c < 3; c++){
        channels[c] = (channels[c] - mean[c]) / stddev[c];
    }
    // combine 3 channels and write to result 
    cv::merge(channels, result);

    // for illustration purposes
    cv::Mat save_normal;
    result.convertTo(save_normal, CV_8UC3, 255.0);
    cv::imwrite("Normalized.jpeg", save_normal);
};


cv::Mat preprocess_img(cv::Mat& src_im, cv::Size sz = cv::Size(224, 224), bool normalize = 1){
    /* 
     * resize image with cubic interpolation, then normalize
     * */
    cv::Mat resized_img;
    cv::resize(src_im, resized_img, sz, cv::INTER_CUBIC);
    cv::Mat final;

    // resize 
    cv::Mat tmp_img;
    resized_img.convertTo(tmp_img, CV_32FC3, 1.0/255.0);

    if (normalize){
        // normalize_img(tmp_img, final);
        final = im_normalize(tmp_img);

        if (DEBUG){
            cv::imwrite("after_resize.jpeg", resized_img);
            cv::Mat final_save;
            final.convertTo(final_save, CV_8UC3, 255.0);
            cv::imwrite("after_preprocess.jpeg", final_save);
        }
        return final;
    }

    else return tmp_img;
};
 
int main() {
    // create websocket server & onnx session
    utility_server s;
    auto onnx_sess = ResNetSession();
    onnx_sess.getInputAndOutputNames();

    // read image from disk & preprocess image
    auto img = cv::imread("../test_img_dog.jpeg", cv::IMREAD_COLOR_RGB);
    img = preprocess_img(img, cv::Size(224, 224), 1);
    std::cout << "Size: " << img.total() << "\t Channels: " << img.channels();
    std::cout << "\n Dims: " << img.size << std::endl;

    // run model on image
    auto res = onnx_sess.detect(img);
    std::cout << "Result: " << res << std::endl;

    // set message handler for server & run server
    s.set_message_handler([&s, &onnx_sess](websocketpp::connection_hdl hdl, server::message_ptr msg){outside_handler(s, onnx_sess,hdl, msg);});
    s.run();
    return 0;
}
