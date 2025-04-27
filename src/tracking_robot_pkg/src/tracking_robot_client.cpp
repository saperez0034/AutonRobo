#include <inttypes.h>
#include <memory>
#include <string>
#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include "custom_interfaces/action/tracking_robot.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "std_msgs/msg/string.hpp"

class TrackingRobotActionClient : public rclcpp::Node
{
public:
  using TrackingRobot = custom_interfaces::action::TrackingRobot;
  using GoalHandleTrackingRobot = rclcpp_action::ClientGoalHandle<TrackingRobot>;

  explicit TrackingRobotActionClient(const rclcpp::NodeOptions & node_options = rclcpp::NodeOptions())
  : Node("tracking_robot_action_client", node_options), goal_done_(false)
  {
    this->client_ptr_ = rclcpp_action::create_client<TrackingRobot>(
      this->get_node_base_interface(),
      this->get_node_graph_interface(),
      this->get_node_logging_interface(),
      this->get_node_waitables_interface(),
      "tracking");
    subscription_ = this->create_subscription<std_msgs::msg::String>(
      "object", 10, std::bind(&TrackingRobotActionClient::input_callback, this, std::placeholders::_1));
  }

  bool is_goal_done() const
  {
    return this->goal_done_;
  }

  void send_goal(const std_msgs::msg::String::SharedPtr msg)
  {
    using namespace std::placeholders;
    this->goal_done_ = false;
    if (!this->client_ptr_) {
      RCLCPP_ERROR(this->get_logger(), "Action client not initialized");
    }
    if (!this->client_ptr_->wait_for_action_server(std::chrono::seconds(10))) {
      RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
      this->goal_done_ = true;
      return;
    }
    auto goal_msg = TrackingRobot::Goal();
    goal_msg.object = msg->data.c_str();
    RCLCPP_INFO(this->get_logger(), "Sending goal");
    auto send_goal_options = rclcpp_action::Client<TrackingRobot>::SendGoalOptions();

    send_goal_options.goal_response_callback =
      std::bind(&TrackingRobotActionClient::goal_response_callback, this, _1);
    send_goal_options.feedback_callback =
      std::bind(&TrackingRobotActionClient::feedback_callback, this, _1, _2);
    send_goal_options.result_callback =
      std::bind(&TrackingRobotActionClient::result_callback, this, _1);

    auto goal_handle_future = this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
  }

private:
  rclcpp_action::Client<TrackingRobot>::SharedPtr client_ptr_;
  rclcpp::TimerBase::SharedPtr timer_;
  bool goal_done_;
  GoalHandleTrackingRobot::SharedPtr goal_handle_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;

  void input_callback(const std_msgs::msg::String::SharedPtr msg)
  {
    RCLCPP_INFO(this->get_logger(), "Received object: %s", msg->data.c_str());
    this->send_goal(msg);
  }

  void goal_response_callback(const GoalHandleTrackingRobot::SharedPtr & goal_handle)
  {
    if (!goal_handle) {
      RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
    } else {
      RCLCPP_INFO(this->get_logger(), "Goal accepted by server, waiting for result");
      this->goal_handle_ = goal_handle;
    }
  }

  void feedback_callback(GoalHandleTrackingRobot::SharedPtr, const std::shared_ptr<const TrackingRobot::Feedback> feedback)
  {
    if (!strcmp(feedback->feedback.c_str(), "Lost object"))
      this->client_ptr_->async_cancel_all_goals();
    RCLCPP_INFO(this->get_logger(), "Feedback received: %s", feedback->feedback.c_str());
  }

  void result_callback(const GoalHandleTrackingRobot::WrappedResult & result)
  {
    this->goal_done_ = true;
    RCLCPP_INFO(this->get_logger(), "Result received: %s", result.result->result.c_str());
    switch (result.code) {
      case rclcpp_action::ResultCode::SUCCEEDED:
        RCLCPP_INFO(this->get_logger(), "Mission Accomplished");
        return;
      case rclcpp_action::ResultCode::ABORTED:
        RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
        return;
      case rclcpp_action::ResultCode::CANCELED:
        RCLCPP_ERROR(this->get_logger(), "Action canceled. The robot lost the object");
        return;
      default:
        RCLCPP_ERROR(this->get_logger(), "Unknown result code");
        return;
    }
  }
};

class InputMonitorNode : public rclcpp::Node
{
public:
  InputMonitorNode() : Node("input_monitor_node")
  {
    RCLCPP_INFO(this->get_logger(), "Node started. Type input below:");
    publisher_ = this->create_publisher<std_msgs::msg::String>("object", 10);
    input_thread_ = std::thread(&InputMonitorNode::readInput, this);
  }

  ~InputMonitorNode()
  {
    keep_running_ = false;
    if (input_thread_.joinable()) {
      input_thread_.join();
    }
  }

private:
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;

  void readInput()
  {
    std::string input;
    while (rclcpp::ok() && keep_running_) {
      std::cout << ">>> " << std::flush;
      std::getline(std::cin, input);
      if (!input.empty()) {
        RCLCPP_INFO(this->get_logger(), "User typed: '%s'", input.c_str());
        std_msgs::msg::String msg;
        msg.data = input;
        publisher_->publish(msg);
      }
    }
  }

  std::thread input_thread_;
  bool keep_running_ = true;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto action_client = std::make_shared<TrackingRobotActionClient>();
  auto input_monitor_node = std::make_shared<InputMonitorNode>();

  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(action_client);
  executor.add_node(input_monitor_node);
  while (!action_client->is_goal_done()) {
    executor.spin();
  }
  rclcpp::shutdown();
  return 0;
}
