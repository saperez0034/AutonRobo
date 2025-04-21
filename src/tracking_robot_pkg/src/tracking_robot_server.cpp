#include <functional>
#include <memory>
#include <thread>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "custom_interfaces/action/tracking_robot.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "vision_msgs/msg/detection2_d_array.hpp"
#include "std_msgs/msg/bool.hpp"
#include <string>
#include <iostream>
#include <array>
#include <mutex>
#include <unordered_set>
#include <algorithm>
#include <cctype>

bool isCocoObject(const std::string& input) {
    // List of all 80 COCO object classes (lowercased) :contentReference[oaicite:0]{index=0}
    static const std::unordered_set<std::string> cocoClasses = {
        "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck",
        "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
        "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra",
        "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
        "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove",
        "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup",
        "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange",
        "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
        "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse",
        "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",
        "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
        "hair drier", "toothbrush"
    };

    // Normalize to lowercase for case-insensitive matching
    std::string s = input;
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    return cocoClasses.find(s) != cocoClasses.end();
}

class TrackingRobotActionServer : public rclcpp::Node
{
public:
    using TrackingRobot = custom_interfaces::action::TrackingRobot;

    using GoalHandleTrackingRobot = rclcpp_action::ServerGoalHandle<TrackingRobot>;

    explicit TrackingRobotActionServer(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
    : Node("tracking_robot_action_server", options)
    {
        using namespace std::placeholders;
        this->action_server_ = rclcpp_action::create_server<TrackingRobot>(
        this,
        "tracking",
        std::bind(&TrackingRobotActionServer::handle_goal, this, _1, _2),
        std::bind(&TrackingRobotActionServer::handle_cancel, this, _1),
        std::bind(&TrackingRobotActionServer::handle_accepted, this, _1));
        RCLCPP_INFO(this->get_logger(), "SERVER STARTED");
        // publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
        subscription_ = this->create_subscription<vision_msgs::msg::Detection2DArray>(
              "detections_output", 10, std::bind(&TrackingRobotActionServer::realsense_callback, this, _1));
    }
  
private:

    std::string feedback_message;
    std::string object_name = "";
    bool object_detected = false;
    bool object_detected_prev = false;
    bool not_done = true;
    double bbox_center_x = 0.0;
    double bbox_center_y = 0.0;

    rclcpp_action::Server<TrackingRobot>::SharedPtr action_server_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::Subscription<vision_msgs::msg::Detection2DArray>::SharedPtr subscription_;

    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const TrackingRobot::Goal> goal)
    {
        RCLCPP_INFO(this->get_logger(), "Received goal request with %s object to track", goal->object.c_str());
        (void)uuid;
        if (isCocoObject(goal->object)) {
            RCLCPP_INFO(this->get_logger(), "Goal accepted");
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        } else {
            RCLCPP_ERROR(this->get_logger(), "Goal rejected");
            return rclcpp_action::GoalResponse::REJECT;
        }
    }

    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<GoalHandleTrackingRobot> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Action canceled.");
        (void)goal_handle;
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accepted(const std::shared_ptr<GoalHandleTrackingRobot> goal_handle)
    {
        using namespace std::placeholders;
        std::thread{std::bind(&TrackingRobotActionServer::execute, this, _1), goal_handle}.detach();
    }

    void execute(const std::shared_ptr<GoalHandleTrackingRobot> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Executing goal");
        const auto goal = goal_handle->get_goal();
        object_name = goal->object;
        auto feedback = std::make_shared<TrackingRobot::Feedback>();
        auto & message = feedback->feedback;
        auto result = std::make_shared<TrackingRobot::Result>();
        rclcpp::Rate loop_rate(1);
        while(rclcpp::ok() && not_done){
            // Check if there is a cancel request
            if (goal_handle->is_canceling()) {
                goal_handle->canceled(result);
                return;
            }
            if (object_detected) {
                if (bbox_center_x < 30 && bbox_center_x > -30 && bbox_center_y < 30 && bbox_center_y > -30){
                    feedback_message = "center";
                    not_done = false;
                    result->result = "Tracking Successful";
                }
                else if (bbox_center_x < -30 && bbox_center_y < 30 && bbox_center_y > -30)
                    feedback_message = "left";
                else if (bbox_center_x > 30 && bbox_center_y < 30 && bbox_center_y > -30)
                    feedback_message = "right";
                else if (bbox_center_x < 30 && bbox_center_x > -30 && bbox_center_y > 30)
                    feedback_message = "top";
                else if (bbox_center_x < 30 && bbox_center_x > -30 && bbox_center_y < -30)
                    feedback_message = "bottom";
                else if (bbox_center_x < -30 && bbox_center_y > 30)
                    feedback_message = "top left";
                else if (bbox_center_x > 30 && bbox_center_y > 30)
                    feedback_message = "top right";
                else if (bbox_center_x < -30 && bbox_center_y < -30)
                    feedback_message = "bottom left";
                else if (bbox_center_x > 30 && bbox_center_y < -30)
                    feedback_message = "bottom right";
                object_detected_prev = true;
            }
            else if (!object_detected && !object_detected_prev) {
                feedback_message = "Searching";
            }
            else if (!object_detected && object_detected_prev) {
                feedback_message = "Lost object";
                not_done = false;
                result->result = "Tracking Failed";
            }
            message = feedback_message;
            goal_handle->publish_feedback(feedback);
            loop_rate.sleep();
        }
        // Check if goal is done
        if (rclcpp::ok()) {
            goal_handle->succeed(result);
            RCLCPP_INFO(this->get_logger(), "Goal succeeded");
        }
    }

    void realsense_callback(const vision_msgs::msg::Detection2DArray::SharedPtr detection_p) {
        uint detection_count = detection_p ->detections.size();
        for (uint i = 0; i < detection_count; i++) {
            vision_msgs::msg::ObjectHypothesisWithPose *results =detection_p->detections[i].results.data();
            std::string id = results->hypothesis.class_id;
            if (id == object_name) {
                // Check if the object is detected
                object_detected = true;
                bbox_center_x = detection_p->detections[i].bbox.center.position.x;
                bbox_center_y = detection_p->detections[i].bbox.center.position.y;
                return;
            }
            else {
                object_detected = false;
            }
        }
    }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto tracking_robot_action_server = std::make_shared<TrackingRobotActionServer>();
    
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(tracking_robot_action_server);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}