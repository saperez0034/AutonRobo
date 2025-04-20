
#include <inttypes.h>
#include <memory>
#include <string>
#include <iostream>
#include <cstring>
#include "custom_interfaces/action/circle_wall.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

class CircleWallActionClient : public rclcpp::Node
{
public:
  using CircleWall = custom_interfaces::action::CircleWall;
  using GoalHandleCircleWall = rclcpp_action::ClientGoalHandle<CircleWall>;

  explicit CircleWallActionClient(const rclcpp::NodeOptions & node_options = rclcpp::NodeOptions())
  : Node("circle_wall_action_client", node_options), goal_done_(false)
  {
    this->client_ptr_ = rclcpp_action::create_client<CircleWall>(
      this->get_node_base_interface(),
      this->get_node_graph_interface(),
      this->get_node_logging_interface(),
      this->get_node_waitables_interface(),
      "move_robot");
    this->timer_ = this->create_wall_timer(
      std::chrono::milliseconds(500),
      std::bind(&CircleWallActionClient::send_goal, this));
  }

  bool is_goal_done() const
  {
    return this->goal_done_;
  }

  void send_goal()
  {
    using namespace std::placeholders;
    this->timer_->cancel();
    this->goal_done_ = false;
    if (!this->client_ptr_) {
      RCLCPP_ERROR(this->get_logger(), "Action client not initialized");
    }
    if (!this->client_ptr_->wait_for_action_server(std::chrono::seconds(10))) {
      RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting");
      this->goal_done_ = true;
      return;
    }
    auto goal_msg = CircleWall::Goal();
    goal_msg.circles = 2;
    RCLCPP_INFO(this->get_logger(), "Sending goal");
    auto send_goal_options = rclcpp_action::Client<CircleWall>::SendGoalOptions();
                
    send_goal_options.goal_response_callback =
      std::bind(&CircleWallActionClient::goal_response_callback, this, _1);
    send_goal_options.feedback_callback =
      std::bind(&CircleWallActionClient::feedback_callback, this, _1, _2);
    send_goal_options.result_callback =
      std::bind(&CircleWallActionClient::result_callback, this, _1);
      
    auto goal_handle_future = this->client_ptr_->async_send_goal(goal_msg, send_goal_options);
  }

private:
  rclcpp_action::Client<CircleWall>::SharedPtr client_ptr_;
  rclcpp::TimerBase::SharedPtr timer_;
  bool goal_done_;
  GoalHandleCircleWall::SharedPtr goal_handle_;

  void goal_response_callback(const GoalHandleCircleWall::SharedPtr & goal_handle)
  {
    if (!goal_handle) {
      RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
    } else {
      RCLCPP_INFO(this->get_logger(), "Goal accepted by server, waiting for result");
      this->goal_handle_ = goal_handle;
    }
  }

  void feedback_callback(
    GoalHandleCircleWall::SharedPtr,
    const std::shared_ptr<const CircleWall::Feedback> feedback)
  {
    if (!strcmp(feedback->feedback.c_str(), "The robot touched the wall."))
      this->client_ptr_->async_cancel_all_goals();
    RCLCPP_INFO(
        this->get_logger(), "Feedback received: %s", feedback->feedback.c_str());
  }
  
  void result_callback(const GoalHandleCircleWall::WrappedResult & result)
  {
    this->goal_done_ = true;
    switch (result.code) {
      case rclcpp_action::ResultCode::SUCCEEDED:
        RCLCPP_INFO(this->get_logger(), "Mission Accomplished");
        return;
      case rclcpp_action::ResultCode::ABORTED:
        RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
        return;
      case rclcpp_action::ResultCode::CANCELED:
        RCLCPP_ERROR(this->get_logger(), "Action canceled. The robot touched the wall.");
        return;
      default:
        RCLCPP_ERROR(this->get_logger(), "Unknown result code");
        return;
    }
    RCLCPP_INFO(this->get_logger(), "Result received: %s", result.result->result.c_str());
  }
}; 

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto action_client = std::make_shared<CircleWallActionClient>();
    
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(action_client);
  while (!action_client->is_goal_done()) {
    executor.spin();
  }
  rclcpp::shutdown();
  return 0;
}