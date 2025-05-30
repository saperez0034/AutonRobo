cmake_minimum_required(VERSION 3.8)
project(topic_publisher_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
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
add_executable(simple_publisher_node src/simple_topic_publisher.cpp)
add_executable(move_robot src/move_robot.cpp)
add_executable(simple_publisher src/simple_publisher.cpp)
add_executable(simple_subscriber src/simple_subscriber.cpp)
add_executable(circle_wall src/circle_wall.cpp)
ament_target_dependencies(simple_publisher_node rclcpp std_msgs)
ament_target_dependencies(move_robot rclcpp std_msgs geometry_msgs)
ament_target_dependencies(simple_publisher rclcpp std_msgs)
ament_target_dependencies(simple_subscriber rclcpp std_msgs)
ament_target_dependencies(circle_wall rclcpp std_msgs sensor_msgs geometry_msgs)

install(TARGETS
	simple_publisher_node
	move_robot
	simple_publisher
	simple_subscriber
  circle_wall
	DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY
	launch
	DESTINATION share/${PROJECT_NAME}/
)
ament_package()
