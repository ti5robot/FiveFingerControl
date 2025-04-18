#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32_multi_array.hpp>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

class KeyboardPublisher : public rclcpp::Node
{
public:
    KeyboardPublisher() : Node("keyboard_publisher")
    {
        publisher_ = this->create_publisher<std_msgs::msg::Int32MultiArray>("hand_command", 10);
        RCLCPP_INFO(this->get_logger(), "输入6个数字，用空格分隔，按Enter键发布(q退出)");
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&KeyboardPublisher::timer_callback, this));
    }

private:
    void timer_callback()
    {
        std::string input;
        std::getline(std::cin, input);

        if (input == "q" || input == "Q")
        {
            rclcpp::shutdown();
            return;
        }

        std::istringstream iss(input);
        std::vector<int> numbers;
        int num;

        while (iss >> num && numbers.size() < 6)
        {
            numbers.push_back(num);
        }

        if (numbers.size() == 6)
        {
            auto msg = std_msgs::msg::Int32MultiArray();
            msg.data = numbers;
            publisher_->publish(msg);
            RCLCPP_INFO(this->get_logger(), "发布: %d %d %d %d %d %d",
                       numbers[0], numbers[1], numbers[2],
                       numbers[3], numbers[4], numbers[5]);
        }
        else
        {
            RCLCPP_WARN(this->get_logger(), "请输入6个数字，用空格分隔");
        }
    }

    rclcpp::Publisher<std_msgs::msg::Int32MultiArray>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KeyboardPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}