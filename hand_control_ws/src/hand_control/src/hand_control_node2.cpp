#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32_multi_array.hpp>
#include <std_msgs/msg/string.hpp> // 添加 String 头文件
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <memory>

using namespace std;
using std::placeholders::_1;

class RS485Controller
{
private:
    int serial_port;

    // 串口初始化
    bool setup_serial(const std::string &port, int baudrate)
    {
        serial_port = open(port.c_str(), O_RDWR | O_NOCTTY);
        if (serial_port < 0)
        {
            RCLCPP_ERROR(rclcpp::get_logger("RS485Controller"), "无法打开串口 %s", port.c_str());
            return false;
        }

        struct termios tty;
        memset(&tty, 0, sizeof(tty));

        if (tcgetattr(serial_port, &tty) != 0)
        {
            RCLCPP_ERROR(rclcpp::get_logger("RS485Controller"), "获取终端属性失败");
            return false;
        }

        // 设置波特率
        cfsetospeed(&tty, baudrate);
        cfsetispeed(&tty, baudrate);

        // 8位数据，无校验，1停止位
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;
        tty.c_cflag &= ~CRTSCTS;       // 禁用硬件流控
        tty.c_cflag |= CREAD | CLOCAL; // 启用接收，忽略调制解调器控制线

        // 非规范模式
        tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // 禁用软件流控
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

        // 原始输出
        tty.c_oflag &= ~OPOST;
        tty.c_oflag &= ~ONLCR;

        // 设置超时：1秒
        tty.c_cc[VMIN] = 0;
        tty.c_cc[VTIME] = 10; // 1秒超时

        if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
        {
            RCLCPP_ERROR(rclcpp::get_logger("RS485Controller"), "设置终端属性失败");
            return false;
        }

        return true;
    }

public:
    RS485Controller(const std::string &port = "/dev/ttyUSB0", int baudrate = B115200)
    {
        if (!setup_serial(port, baudrate))
        {
            throw std::runtime_error("串口初始化失败");
        }
        RCLCPP_INFO(rclcpp::get_logger("RS485Controller"), "RS485控制器初始化成功，端口: %s", port.c_str());
    }

    ~RS485Controller()
    {
        if (serial_port >= 0)
        {
            close(serial_port);
            RCLCPP_INFO(rclcpp::get_logger("RS485Controller"), "RS485端口已关闭");
        }
    }

    bool tempature()
    {
        uint8_t data[6] = {1, 2, 3, 4, 5, 6};

        for (int i = 0; i < 6; i++)
        {
            std::vector<uint8_t> command = {
                0xFF, 0xFF,    // 帧头
                0xd1, 0x01,    // 地址和模式
                data[i], 0x01, // id
                0x00, 0x00,
                0x00, 0x00,
                0x00, 0x00,
                0x00, 0x00,
                0x00, 0x00,
                0xFF, 0xFF // 帧尾
            };

            std::vector<uint8_t> response;
            if (!send_command(command, response))
            {
                return false;
            }
            int a = response[7];
            if(a>=78)
            {
                return true;
            }
        }
        return false;
    }

    bool postion()
    {
        uint8_t data[6] = {1, 2, 3, 4, 5, 6};

        for (int i = 0; i < 6; i++)
        {
            std::vector<uint8_t> command = {
                0xFF, 0xFF,    // 帧头
                0xd1, 0x01,    // 地址和模式
                data[i], 0x01, // id
                0x00, 0x00,
                0x00, 0x00,
                0x00, 0x00,
                0x00, 0x00,
                0x00, 0x00,
                0xFF, 0xFF // 帧尾
            };

            std::vector<uint8_t> response;
            if (!send_command(command, response))
            {
                return false;
            }
            int a = response[7];
            if(a>=78)
            {
                return true;
            }
        }
        return false;
    }

    // 构建18字节命令帧
    std::vector<uint8_t> build_command(uint8_t address, uint8_t mode,
                                       const std::vector<uint8_t> &data)
    {
        std::vector<uint8_t> command = {
            0xFF, 0xFF,       // 帧头
            address, mode,    // 地址和模式
            data[0], 0xff,    // id
            0x64,             // 加速度
            data[2], 0x00,    // 位置
            0x00, 0x00,       // 时间
            data[3], data[4], // 速度
            0xAC, 0x02,       // 转矩
            0x00,
            0xFF, 0xFF // 帧尾
        };

        return command;
    }

    // 发送命令并接收响应
    bool send_command(const std::vector<uint8_t> &command, std::vector<uint8_t> &response)
    {
        // 发送完整18字节帧
        ssize_t written = write(serial_port, command.data(), command.size());
        if (written != command.size())
        {
            RCLCPP_ERROR(rclcpp::get_logger("RS485Controller"), "命令写入不完整");
            return false;
        }

        // 等待数据发送完成
        tcdrain(serial_port);

        // 读取响应（假设也是18字节）
        response.resize(18);
        ssize_t read_bytes = read(serial_port, response.data(), response.size());

        if (read_bytes != 18)
        {
            RCLCPP_WARN(rclcpp::get_logger("RS485Controller"), "响应不完整: %zd 字节", read_bytes);
            return false;
        }

        RCLCPP_DEBUG(rclcpp::get_logger("RS485Controller"), "命令发送成功，收到响应");
        return true;
    }

    // 设置舵机位置
    bool set_servo_position(uint8_t Command, uint8_t *msg, uint16_t speed = 950)
    {
        if (Command == 3)
        {
            speed = 1000;
            uint8_t speedlow_byte = speed & 0xFF;
            uint8_t speedhigh_byte = (speed >> 8) & 0xFF;

            for (int i = 0; i < 6; i++)
            {
                uint8_t position = static_cast<uint8_t>(msg[i]);

                // if (position <= 10 && position >= 0)
                // {
                //     position = 0;
                // }
                // else if (position <= 100 && position >= 90)
                // {
                //     position = 90;
                // }
                // else
                // {
                //     position = position - 10;
                // }

                std::vector<uint8_t> data = {
                    uint8_t(i + 1), // ID
                    0x01,           // 写操作
                    position,       // 位置
                    speedlow_byte,
                    speedhigh_byte, // 速度
                    0               // 时间
                };

                auto cmd = build_command(0x03, 0xA1, data);
                std::vector<uint8_t> response;
                if (!send_command(cmd, response))
                {
                    return false;
                }
                usleep(5000);
            }
        }
        return true;
    }
};

class HandControlNode : public rclcpp::Node
{
public:
    HandControlNode() : Node("hand_control_node")
    {
        try
        {
            controller_ = std::make_unique<RS485Controller>("/dev/ttyUSB1");
            subscription_ = this->create_subscription<std_msgs::msg::String>(
                "yudiehand_angle", 10,
                std::bind(&HandControlNode::command_callback, this, _1));

            RCLCPP_INFO(this->get_logger(), "手部控制节点准备就绪，等待命令...");
        }
        catch (const std::exception &e)
        {
            RCLCPP_FATAL(this->get_logger(), "初始化失败: %s", e.what());
            rclcpp::shutdown();
        }
    }

private:
    void command_callback(const std_msgs::msg::String::SharedPtr msg)
    {
        if (!controller_)
            return;

        // 定义两个固定大小的数组存储左右手数据
        uint8_t left_hand[6] = {0};
        uint8_t right_hand[6] = {0};

        // 临时变量存储解析的数据
        char finger_name[200];
        float x, y, z;

        // 使用C风格字符串处理提高效率
        const char *data = msg->data.c_str();
        const char *line_start = data;

        while (*line_start != '\0')
        {
            // 查找行结束位置
            const char *line_end = strchr(line_start, '\n');
            if (!line_end)
                line_end = line_start + strlen(line_start);

            // 解析行数据
            if (sscanf(line_start, "%[^:]: (%f,%f,%f)", finger_name, &x, &y, &z) == 4)
            {
                // 处理左手数据
                if (strstr(finger_name, "Left") != nullptr)
                {
                    if (strstr(finger_name, "Pinky3"))
                        left_hand[0] = abs(static_cast<int>(x));
                    else if (strstr(finger_name, "Ring3"))
                        left_hand[1] = abs(static_cast<int>(x));
                    else if (strstr(finger_name, "Middle3"))
                        left_hand[2] = abs(static_cast<int>(x));
                    else if (strstr(finger_name, "Index3"))
                        left_hand[3] = abs(static_cast<int>(x));
                    else if (strstr(finger_name, "Thumb3"))
                        left_hand[4] = abs(static_cast<int>(x));
                    else if (strstr(finger_name, "Thumb1"))
                        left_hand[5] = abs(static_cast<int>(y));
                }
                // 处理右手数据
                else if (strstr(finger_name, "Right") != nullptr)
                {
                    if (strstr(finger_name, "Pinky3"))
                        right_hand[0] = abs(static_cast<int>(x));
                    else if (strstr(finger_name, "Ring3"))
                        right_hand[1] = abs(static_cast<int>(x));
                    else if (strstr(finger_name, "Middle3"))
                        right_hand[2] = abs(static_cast<int>(x));
                    else if (strstr(finger_name, "Index3"))
                        right_hand[3] = abs(static_cast<int>(x));
                    else if (strstr(finger_name, "Thumb3"))
                        right_hand[4] = abs(static_cast<int>(x));
                    else if (strstr(finger_name, "Thumb1"))
                        right_hand[5] = abs(static_cast<int>(y));
                }
            }
            

            // 移动到下一行
            line_start = (*line_end == '\0') ? line_end : line_end + 1;
        }

        for (int i = 0; i < 6; i++)
        {
            int num = right_hand[i];
            cout << "数字为" << num << endl;
        }

        cout << "---------------------------------------------------------------------------------------------" << endl;

        int a = 1;
        if (a == 1)
        {
            // 调用原来的控制逻辑
            controller_->set_servo_position(3, right_hand, 950);
        }
        else if (a == 2)
        {
            controller_->set_servo_position(3, left_hand, 950);
        }
    }

    std::unique_ptr<RS485Controller> controller_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<HandControlNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
