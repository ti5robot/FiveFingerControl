# cmake_minimum_required(VERSION 3.8)
# project(hand_control)

# if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#   add_compile_options(-Wall -Wextra -Wpedantic)
# endif()

# # find dependencies
# find_package(ament_cmake REQUIRED)
# find_package(rclcpp REQUIRED)
# find_package(std_msgs REQUIRED)

# if(BUILD_TESTING)
#   find_package(ament_lint_auto REQUIRED)
#   # the following line skips the linter which checks for copyrights
#   # comment the line when a copyright and license is added to all source files
#   set(ament_cmake_copyright_FOUND TRUE)
#   # the following line skips cpplint (only works in a git repo)
#   # comment the line when this package is in a git repo and when
#   # a copyright and license is added to all source files
#   set(ament_cmake_cpplint_FOUND TRUE)
#   ament_lint_auto_find_test_dependencies()
# endif()

# ament_package()

cmake_minimum_required(VERSION 3.8)
project(hand_control)

# 默认使用C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

# 查找依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# 添加hand_control_node可执行文件
add_executable(hand_control_node2 
  src/hand_control_node2.cpp)
target_include_directories(hand_control_node2 PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(hand_control_node2 PUBLIC c_std_99 cxx_std_14)
ament_target_dependencies(hand_control_node2
  "rclcpp"
  "std_msgs")


# 添加hand_control_node可执行文件
add_executable(hand_control_node 
  src/hand_control_node.cpp)
target_include_directories(hand_control_node PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(hand_control_node PUBLIC c_std_99 cxx_std_14)
ament_target_dependencies(hand_control_node
  "rclcpp"
  "std_msgs")

# 添加keyboard_publisher可执行文件
add_executable(keyboard_publisher 
  src/keyboard_publisher.cpp)
target_include_directories(keyboard_publisher PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(keyboard_publisher PUBLIC c_std_99 cxx_std_14)
ament_target_dependencies(keyboard_publisher
  "rclcpp"
  "std_msgs")

# 安装可执行文件
install(TARGETS
  hand_control_node
  hand_control_node2
  keyboard_publisher
  DESTINATION lib/${PROJECT_NAME})

# 导出依赖
ament_export_dependencies(
  rclcpp
  std_msgs)

ament_package()
