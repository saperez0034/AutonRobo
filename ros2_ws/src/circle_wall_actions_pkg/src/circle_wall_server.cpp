
#include <functional>
#include <memory>
#include <thread>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "custom_interfaces/action/circle_wall.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_msgs/msg/bool.hpp"
#include <iostream>
#include <array>
#include <mutex>

class CircleWallActionServer : public rclcpp::Node
{
public:
    using Circle = custom_interfaces::action::CircleWall;

    using GoalHandleCircleWall = rclcpp_action::ServerGoalHandle<Circle>;

    explicit CircleWallActionServer(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
    : Node("circle_wall_action_server", options)
    {
        using namespace std::placeholders;
        this->action_server_ = rclcpp_action::create_server<Circle>(
        this,
        "move_robot",
        std::bind(&CircleWallActionServer::handle_goal, this, _1, _2),
        std::bind(&CircleWallActionServer::handle_cancel, this, _1),
        std::bind(&CircleWallActionServer::handle_accepted, this, _1));
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
        subscription1_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "lidar", 10, std::bind(&CircleWallActionServer::lidar_callback, this, _1));
        subscription2_ = this->create_subscription<std_msgs::msg::Bool>(
            "wall/touched", 10, std::bind(&CircleWallActionServer::wall_callback, this, _1));
    }
  
private:

    enum state {
        APPROACH = 0,
        TURN_RIGHT = 1,
        MOVE_ALONG = 2,
        TURN_LEFT_WALL = 3,
        ENDED = 4,
        TOUCHED_WALL = 5
    };

    uint32_t circles = 0;
    uint32_t turns = 0;
    int currentState = APPROACH;
    const int SCAN_SIZE = 640;
    bool wall_touched = false;
    std::mutex touched_mutex;
    std::mutex turn_mutex;
    std::string feedback_message;

    rclcpp_action::Server<Circle>::SharedPtr action_server_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription1_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr subscription2_;

    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const Circle::Goal> goal)
    {
        RCLCPP_INFO(this->get_logger(), "Received goal request with %d circles around wall", goal->circles);
        (void)uuid;
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<GoalHandleCircleWall> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Action canceled.");
        (void)goal_handle;
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accepted(const std::shared_ptr<GoalHandleCircleWall> goal_handle)
    {
        using namespace std::placeholders;
        // this needs to return quickly to avoid blocking the executor, so spin up a new thread
        std::thread{std::bind(&CircleWallActionServer::execute, this, _1), goal_handle}.detach();
    }

    void execute(const std::shared_ptr<GoalHandleCircleWall> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Executing goal");
        const auto goal = goal_handle->get_goal();
        auto feedback = std::make_shared<Circle::Feedback>();
        auto & message = feedback->feedback;
        message = "Starting movement...";
        auto result = std::make_shared<Circle::Result>();
        auto move = geometry_msgs::msg::Twist();
        rclcpp::Rate loop_rate(1);
        while(circles < goal->circles && rclcpp::ok()){
            // Check if there is a cancel request
            if (goal_handle->is_canceling()) {
                goal_handle->canceled(result);
                return;
            }
            { 
                turn_mutex.lock();
                circles = turns / 2;
                turn_mutex.unlock();
            }
            {
                touched_mutex.lock();
                if (wall_touched){
                    feedback_message = "The robot touched the wall.";
                    currentState = TOUCHED_WALL;
                }
                touched_mutex.unlock();
            }
            message = feedback_message;
            goal_handle->publish_feedback(feedback);
            loop_rate.sleep();
        }
        // Check if goal is done
        if (rclcpp::ok()) {
            currentState = ENDED;
            publisher_->publish(move);
            goal_handle->succeed(result);
            RCLCPP_INFO(this->get_logger(), "Goal succeeded");
        }
    }

    void wall_callback(const std_msgs::msg::Bool::SharedPtr msg) {
        if (msg->data) {
            touched_mutex.lock();
            wall_touched = true;
            touched_mutex.unlock();
        }
    }

    void lidar_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
        auto move = geometry_msgs::msg::Twist();
            switch (currentState){
            case APPROACH:
                move.linear.x = 1;
                feedback_message = "Approaching";
                if(msg->ranges[320] < 1.0){
                    move.linear.x = 0.0;
                    currentState = TURN_RIGHT;
                }
                break;
            case TURN_RIGHT:
                move.angular.z = -0.3;
                feedback_message = "Turning right";
                if(msg->ranges[320] > 10.0 && msg->ranges[639] > 2.0){
                    move.angular.z = 0.0;
                    move.linear.x = 0.0;
                    currentState = MOVE_ALONG;
                }
                break;
            case MOVE_ALONG:
                move.linear.x = 1.5;
                feedback_message = "Moving";
                if(msg->ranges[639] > 10.0){
                    for (u_int i = 0; i < 0xFFFFFFFF; i++);
                    move.linear.x = 0.0;
                    currentState = TURN_LEFT_WALL;
                }
                if(msg->ranges[639] < 2.0){
                    move.linear.x = 0.0;
                    currentState = TURN_RIGHT;
                }
                break;
            case TURN_LEFT_WALL:
                move.angular.z = 0.3;
                move.linear.x = 0.75;
                feedback_message = "Turning";
                if(msg->ranges[639] < 2.1 && msg->ranges[320] > 10.0){
                    move.angular.z = 0.0;
                    move.linear.x = 0.0;
                    currentState = MOVE_ALONG;
                    {
                        turn_mutex.lock();
                        turns++;
                        turn_mutex.unlock();
                    }
                }
                break;
            case ENDED:
                move.linear.x = 0.0;
                move.angular.z = 0.0;
                break;
            case TOUCHED_WALL:
                move.linear.x = 0.0;
                move.angular.z = 0.0;
                break;
            }
        publisher_->publish(move);
        }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto circle_wall_action_server = std::make_shared<CircleWallActionServer>();
    
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(circle_wall_action_server);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}