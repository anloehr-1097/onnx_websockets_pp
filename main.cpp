#include "websocketpp/common/connection_hdl.hpp"
#include "websocketpp/frame.hpp"
#include <cstddef>
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <string>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <functional>
// #include <onnxruntime_cxx_api.h>
#include "inference.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;
class utility_server;
void outside_handler(utility_server &us, websocketpp::connection_hdl hdl, server::message_ptr msg);


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
        // TODO
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


// outside handler, bit more complicated
void outside_handler(utility_server &us, websocketpp::connection_hdl hdl, server::message_ptr msg) {
    // write a new message
    std::string new_msg_payload;
    if (msg->get_opcode() == websocketpp::frame::opcode::binary){
        new_msg_payload = "This is a bytes frame"; // non const ref to payload
        std::vector<uchar> payload_2(msg->get_payload().begin(), msg->get_payload().end());
        cv::Mat img = cv::imdecode(payload_2, cv::IMREAD_UNCHANGED);
         if (img.empty()) {
                throw std::runtime_error("Failed to decode image from byte string");
        }
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


void normalize_img(cv::Mat &img, cv::Mat &result){
    cv::Scalar mean, std;
    std::vector<cv::Mat> channels(3);
    cv::split(img, channels);
    cv::meanStdDev(img, mean, std);
    std::cout << "Mean: " << mean << "\tStd Dev: " << std << std::endl;
    for (int c = 0; c < 3; c++){
        channels[c] = (channels[c] - mean[c]) / std[c];
    }
    cv::merge(channels, result);
};

 
int main() {
    utility_server s;


    auto onnx_sess = ResNetSession();
    onnx_sess.getInputAndOutputNames();

    // read image from disk
    auto img = cv::imread("../test_img_dog.jpeg", cv::IMREAD_COLOR_RGB);


    // resize image to desired model size
    cv::Mat resized_array;
    cv::Mat pre_process_img;
    cv::resize(img, resized_array, cv::Size(224, 224));
    resized_array.convertTo(pre_process_img, CV_32F, 1.0/255.0);

    // TODO normalize image 
    //

    cv::Mat final_img;
    normalize_img(pre_process_img, final_img);


    float *buffer = final_img.ptr<float>();

    std::cout << "Size: " << final_img.total() << "\t Channels: " << final_img.channels();
    std::cout << "\n Dims: " << final_img.size << std::endl;
    
    onnx_sess.set_input_tensor(buffer, final_img.total() * final_img.channels());
    // print_image("../test_img.jpeg");


    auto res = onnx_sess.Run();
    std::cout << "Result: " << res << std::endl;

    s.set_message_handler([&s](websocketpp::connection_hdl hdl, server::message_ptr msg){outside_handler(s, hdl, msg);});
    s.run();
    return 0;
}
