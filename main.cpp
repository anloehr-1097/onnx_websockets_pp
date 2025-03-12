#include "websocketpp/common/connection_hdl.hpp"
#include "websocketpp/frame.hpp"
#include <opencv2/highgui.hpp>
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

        

        m_endpoint.send(hdl, msg->get_payload(), msg->get_opcode());
    }

    void duplicate_handler(websocketpp::connection_hdl hdl, server::message_ptr msg) {
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
    if (msg->get_opcode() == websocketpp::frame::opcode::binary)
        new_msg_payload = "This is a bytes frame"; // non const ref to payload
    
    else {
        new_msg_payload = "This is a text frame"; // non const ref to payload
    };

    us.send(hdl, new_msg_payload, websocketpp::frame::opcode::text);
    // m_endpoint.send(hdl, new_msg_payload, msg->get_opcode());
}


int print_image(const std::string fpath){
    cv::Mat image = cv::imread(fpath, cv::IMREAD_COLOR);

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



 
int main() {
    utility_server s;


    auto onnx_sess = ResNetSession();
    onnx_sess.print_info();
    print_image("../test_img.jpeg");

    s.set_message_handler([&s](websocketpp::connection_hdl hdl, server::message_ptr msg){outside_handler(s, hdl, msg);});
    s.run();
    return 0;
}
