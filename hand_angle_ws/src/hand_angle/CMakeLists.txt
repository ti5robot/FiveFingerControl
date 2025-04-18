cmake_minimum_required(VERSION 3.8)
project(hand_angle)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(jsoncpp REQUIRED)

# 将UDEServer编译为对象库
add_library(ude_server OBJECT
  libs/UDEServer.cpp
)
target_include_directories(ude_server PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ude_server
  jsoncpp_lib
)

# 主可执行文件
add_executable(hand_angle_node
  src/hand_angle_node.cpp
  $<TARGET_OBJECTS:ude_server>
)
target_include_directories(hand_angle_node PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 关键修改点：使用正确的目标链接方式
target_link_libraries(hand_angle_node
  rclcpp::rclcpp
  ${std_msgs_TARGETS}  # 改为这种方式
  jsoncpp_lib
)

install(TARGETS hand_angle_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()