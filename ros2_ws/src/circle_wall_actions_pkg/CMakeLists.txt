cmake_minimum_required(VERSION 3.8)
project(circle_wall_actions_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(custom_interfaces REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

add_executable(circle_wall_server src/circle_wall_server.cpp)
add_executable(circle_wall_client src/circle_wall_client.cpp)
ament_target_dependencies(circle_wall_server rclcpp rclcpp_action custom_interfaces std_msgs sensor_msgs geometry_msgs)
ament_target_dependencies(circle_wall_client rclcpp rclcpp_action custom_interfaces std_msgs)

install(TARGETS
  circle_wall_server
  circle_wall_client
	DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY
	launch
	DESTINATION share/${PROJECT_NAME}/
)

ament_package()
