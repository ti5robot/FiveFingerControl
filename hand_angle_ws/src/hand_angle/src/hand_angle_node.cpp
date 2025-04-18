#include "rclcpp/rclcpp.hpp"
#include "hand_angle/UDEServer.h"
#include "std_msgs/msg/string.hpp"  // 用于发布字符串消息




class HandAngleWrapper : public rclcpp::Node
{
public:
    HandAngleWrapper() : Node("hand_angle_wrapper"),
        publisher_(this->create_publisher<std_msgs::msg::String>("yudiehand_angle", 10))
    {
        original_main();
    }


    
private:
    void original_main()
    {
        UDEGloveSDK sdk;
        sdk.Initialize();

        sdk.StartListening();
        std::cout << "Server Start Listening.." << std::endl;
        int index = 1;
        Json::Value::Members list;
        bool quit = false;

        while (rclcpp::ok() && !quit)
        {
            try {
                list = sdk.GetRoleNameList();
                if (list.size() > 0) {
                    auto FingerData = sdk.GetVecFingerData(list[0]);
                    auto ControllerMapData = sdk.GetVecControllerData(list[0]);
                    if (sdk.GetStatus() != 2) continue;
                    
                    std::cout << "--------------" << index++ << "-----------" << std::endl;
                    
                    // 创建消息字符串流
                    std::ostringstream msg_stream;
                    msg_stream << "--------------" << (index-1) << "-----------\n";
                    
                    for (int i = 0; i < (int)sizeof(GloveDataHeaders)/sizeof(GloveDataHeaders[0]); ++i) {
                        // 原始打印
                        if(i>14)
                        {
                           std::cout << GloveDataHeaders[i] << ": (" 
                                 << FingerData[i].x << "," 
                                 << FingerData[i].y << "," 
                                 << FingerData[i].z << ")" << std::endl; 
                        }
                        
                        
                        // 添加到消息流
                        msg_stream << GloveDataHeaders[i] << ": (" 
                                 << FingerData[i].x << "," 
                                 << FingerData[i].y << "," 
                                 << FingerData[i].z << ")\n";
                    }
                    
                    std::cout << "-------------------------" << std::endl;
                    msg_stream << "-------------------------\n";
                    
                    // 发布消息
                    auto message = std_msgs::msg::String();
                    message.data = msg_stream.str();
                    publisher_->publish(message);

                    list.clear();
                    if (index >= 7200 * 2000) {
                        std::cout << "End Listening." << index << std::endl;
                        sdk.EndListening();
                        quit = true;
                        break;
                    }
                }
                usleep(1000000 / 120);
            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n';
            }
        }
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<HandAngleWrapper>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}


