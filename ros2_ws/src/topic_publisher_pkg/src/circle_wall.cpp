#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/publisher.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_msgs/msg/int32.hpp"
#include <iostream>
#include <array>

using std::placeholders::_1;
using namespace std;

enum state {
    APPROACH = 0,
    TURN_RIGHT = 1,
    MOVE_ALONG = 2,
    TURN_LEFT_WALL = 3
};

int currentState = APPROACH;
const int SCAN_SIZE = 640;

class CircleWall : public rclcpp::Node {
public:
  CircleWall() : Node("circle_wall_node") {
    subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
        "lidar", 10, std::bind(&CircleWall::topic_callback, this, _1));
    publisher_ = 
        this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
  }

private:
  void topic_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
      auto message = geometry_msgs::msg::Twist();
      switch (currentState){
        case APPROACH:
            message.linear.x = 1;
            if(msg->ranges[320] < 1.0){
                message.linear.x = 0.0;
                currentState = TURN_RIGHT;
            }
            break;
        case TURN_RIGHT:
            message.angular.z = -0.3;
            if(msg->ranges[320] > 10.0 && msg->ranges[639] > 2.0){
                message.angular.z = 0.0;
                message.linear.x = 0.0;
                currentState = MOVE_ALONG;
            }
            break;
        case MOVE_ALONG:
            message.linear.x = 1.5;
            if(msg->ranges[639] > 10.0){
                for (u_int i = 0; i < 0xFFFFFFFF; i++);
                message.linear.x = 0.0;
                currentState = TURN_LEFT_WALL;
            }
            if(msg->ranges[639] < 2.0){
                message.linear.x = 0.0;
                currentState = TURN_RIGHT;
            }
            break;
        case TURN_LEFT_WALL:
            message.angular.z = 0.3;
            message.linear.x = 0.75;
            if(msg->ranges[639] < 2.1 && msg->ranges[320] > 10.0){
                message.angular.z = 0.0;
                message.linear.x = 0.0;
                currentState = MOVE_ALONG;
            }
            break;
      }
      publisher_->publish(message);
  }
    
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CircleWall>());
  rclcpp::shutdown();
  return 0;
}