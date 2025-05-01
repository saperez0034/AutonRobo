#include <functional>
#include <memory>
#include <thread>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "custom_interfaces/action/tracking_robot.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "vision_msgs/msg/detection2_d_array.hpp"
#include "std_msgs/msg/bool.hpp"
#include "sensor_msgs/msg/image.hpp"
#include <string>
#include <iostream>
#include <array>
#include <mutex>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <sensor_msgs/image_encodings.hpp>

#include <message_filters/subscriber.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <message_filters/synchronizer.h>

using std::placeholders::_1;
using namespace std;
using namespace std::chrono_literals;

#define REALSENSE_MIDPT_X 320
#define REALSENSE_MIDPT_Y 240

static const std::unordered_map<std::string, int> coco_class_map = {
  {"person",          0},
  {"bicycle",         1},
  {"car",             2},
  {"motorcycle",      3},
  {"airplane",        4},
  {"bus",             5},
  {"train",           6},
  {"truck",           7},
  {"boat",            8},
  {"traffic light",   9},
  {"fire hydrant",   10},
  {"stop sign",      11},
  {"parking meter",  12},
  {"bench",          13},
  {"bird",           14},
  {"cat",            15},
  {"dog",            16},
  {"horse",          17},
  {"sheep",          18},
  {"cow",            19},
  {"elephant",       20},
  {"bear",           21},
  {"zebra",          22},
  {"giraffe",        23},
  {"backpack",       24},
  {"umbrella",       25},
  {"handbag",        26},
  {"tie",            27},
  {"suitcase",       28},
  {"frisbee",        29},
  {"skis",           30},
  {"snowboard",      31},
  {"sports ball",    32},
  {"kite",           33},
  {"baseball bat",   34},
  {"baseball glove", 35},
  {"skateboard",     36},
  {"surfboard",      37},
  {"tennis racket",  38},
  {"bottle",         39},
  {"wine glass",     40},
  {"cup",            41},
  {"fork",           42},
  {"knife",          43},
  {"spoon",          44},
  {"bowl",           45},
  {"banana",         46},
  {"apple",          47},
  {"sandwich",       48},
  {"orange",         49},
  {"broccoli",       50},
  {"carrot",         51},
  {"hot dog",        52},
  {"pizza",          53},
  {"donut",          54},
  {"cake",           55},
  {"chair",          56},
  {"couch",          57},
  {"potted plant",   58},
  {"bed",            59},
  {"dining table",   60},
  {"toilet",         61},
  {"tv",             62},
  {"laptop",         63},
  {"mouse",          64},
  {"remote",         65},
  {"keyboard",       66},
  {"cell phone",     67},
  {"microwave",      68},
  {"oven",           69},
  {"toaster",        70},
  {"sink",           71},
  {"refrigerator",   72},
  {"book",           73},
  {"clock",          74},
  {"vase",           75},
  {"scissors",       76},
  {"teddy bear",     77},
  {"hair drier",     78},
  {"toothbrush",     79}
};

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

        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

        sub_depth_.subscribe(this, "/depth",   rmw_qos_profile_sensor_data);
        sub_det_.subscribe  (this, "/detections_output", rmw_qos_profile_default);

        sync_ = std::make_shared<Sync>(
            SyncPolicy(10), sub_det_, sub_depth_);
        sync_->registerCallback(
            std::bind(&TrackingRobotActionServer::realsense_callback, this, _1, _2));
    }

private:
    enum class State { SEARCHING, TRACKING, APPROACH, HALT };
    State state_{State::SEARCHING};

    // tuning parameters
    const double search_rot_speed_ = -1.0;       // rad/s during SEARCH
    const double max_ang_speed_    = 1.25;       // rad/s max correction
    const double max_lin_speed_    = 0.4;       // m/s max forward
    const double center_tol_px_    = 30.0;      // px for “centered”
    const double depth_target_     = 0.2;       // m desired distance
    const double depth_tol_        = 0.05;      // m tolerance

    std::string feedback_message;
    std::string object_name = "";
    bool object_detected = false;
    bool object_detected_prev = false;
    int lost_object_count = 0;
    bool not_done = true;
    double bbox_center_x = 0.0;
    double bbox_center_y = 0.0;
    // uint8_t bbox_depth = 0;
    float   bbox_depth = 0.0f;
    int target_class_id_ = -1;
    bool active_ = true;

    rclcpp_action::Server<TrackingRobot>::SharedPtr action_server_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;

    // Synch policy shorthand
    using SyncPolicy = message_filters::sync_policies::ApproximateTime<vision_msgs::msg::Detection2DArray, sensor_msgs::msg::Image>;
    using Sync = message_filters::Synchronizer<SyncPolicy>;

    std::shared_ptr<Sync> sync_;
    message_filters::Subscriber<sensor_msgs::msg::Image>           sub_depth_;
    message_filters::Subscriber<vision_msgs::msg::Detection2DArray> sub_det_;

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
        active_ = false;
        (void)goal_handle;
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accepted(const std::shared_ptr<GoalHandleTrackingRobot> goal_handle)
    {
        using namespace std::placeholders;
        state_              = State::SEARCHING;
        not_done            = true;
        object_detected_prev= false;
        lost_object_count   = 0;
        active_             = true;
        std::thread{std::bind(&TrackingRobotActionServer::execute, this, _1), goal_handle}.detach();
    }

    void execute(const std::shared_ptr<GoalHandleTrackingRobot> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Executing goal");
        const auto goal = goal_handle->get_goal();
        object_name = goal->object;

        // convert object_name → target_class_id_
        auto it = coco_class_map.find(object_name);
        if (it == coco_class_map.end()) {
            RCLCPP_ERROR(this->get_logger(),
                "Unknown object '%s' - aborting", object_name.c_str());
            auto result = std::make_shared<TrackingRobot::Result>();
            result->result = "Unknown object";
            goal_handle->abort(result);
            return;
        }
        target_class_id_ = it->second;
        RCLCPP_INFO(this->get_logger(),
            "Looking for class_id %d (%s)", target_class_id_, object_name.c_str());

        auto feedback = std::make_shared<TrackingRobot::Feedback>();
        auto & message = feedback->feedback;
        auto result = std::make_shared<TrackingRobot::Result>();
        geometry_msgs::msg::Twist cmd;
        rclcpp::Rate loop_rate(2);

        // Start in SEARCHING
        state_ = State::SEARCHING;

        while(rclcpp::ok() && not_done){
            // Check if there is a cancel request
            if (goal_handle->is_canceling()) {
                result->result = "Tracking Failed!";
                goal_handle->canceled(result);
                cmd.linear.x  = 0.0;
                cmd.angular.z  = 0.0;
                publisher_->publish(cmd);
                RCLCPP_INFO(this->get_logger(), "Goal canceled");
                return;
            }

            if (object_detected) {
                // X‐axis only: left, center, or right
                if (bbox_center_x < REALSENSE_MIDPT_X - 30) {
                    feedback_message = "left";
                }
                else if (bbox_center_x > REALSENSE_MIDPT_X + 30) {
                    feedback_message = "right";
                }
                else {
                    feedback_message = "center";
                }
                object_detected_prev = true;
                lost_object_count = 0;
            }
            else if (!object_detected && !object_detected_prev) {
                feedback_message = "Searching";
            }
            else if (!object_detected && object_detected_prev) {
                lost_object_count++;
                if (lost_object_count > 5) {
                    feedback_message = "Lost object";
                    lost_object_count = 0;
                }
                else
                    feedback_message = "Lost object temporarily";
            }

            switch (state_) {
                case State::SEARCHING:
                    RCLCPP_INFO(this->get_logger(), "State: Searching");
                    cmd.linear.x  = 0.05;
                    cmd.angular.z = search_rot_speed_;
                    if (object_detected) {
                        state_ = State::TRACKING;
                    }
                    break;

                case State::TRACKING:
                    RCLCPP_INFO(this->get_logger(), "State: Tracking");
                    if (object_detected) {
                        double err_x     = bbox_center_x - REALSENSE_MIDPT_X;
                        double norm_err  = err_x / REALSENSE_MIDPT_X;
                        cmd.linear.x     = 0.05;
                        cmd.angular.z    = -std::clamp(norm_err * max_ang_speed_,
                                                    -max_ang_speed_, max_ang_speed_);
                        if (std::abs(err_x) < center_tol_px_) {
                            state_ = State::APPROACH;
                        }
                    }
                    break;

                case State::APPROACH:
                    RCLCPP_INFO(this->get_logger(), "State: Approach");
                    if (object_detected) {
                        // If it drifts off‐center, go back to TRACKING
                        double err_x = bbox_center_x - REALSENSE_MIDPT_X;
                        if (std::abs(err_x) > center_tol_px_) {
                            state_ = State::TRACKING;
                            break;
                        }

                        double depth_err = bbox_depth - depth_target_;
                        cmd.angular.z    = 0.0;
                        // proportional forward speed (max at ~0.5 m error)
                        cmd.linear.x     = std::clamp(depth_err,
                                                    0.0, max_lin_speed_);
                        if (std::abs(depth_err) < depth_tol_) {
                            not_done      = false;
                            cmd.linear.x  = 0.0;
                            cmd.angular.z = 0.0;
                            result->result = "Tracking Successful";
                            state_        = State::HALT;
                        }
                    }
                    break;

                case State::HALT:
                    cmd.linear.x  = 0.0;
                    cmd.angular.z = 0.0;
                    break;
            }

            publisher_->publish(cmd);

            message = feedback_message;
            goal_handle->publish_feedback(feedback);
            loop_rate.sleep();
        }
        // Check if goal is done
        if (rclcpp::ok()) {
            goal_handle->succeed(result);
            RCLCPP_INFO(this->get_logger(), "Goal succeeded");
            active_ = false;
            //rclcpp::shutdown();
        }
    }

    void realsense_callback(
        const vision_msgs::msg::Detection2DArray::ConstSharedPtr & detection_p,
        const sensor_msgs::msg::Image::ConstSharedPtr & depth_msg
    ) {
        if(!active_)
            return;

        cv_bridge::CvImageConstPtr cv_depth_ptr;
        try {
          // keep the native encoding (e.g. 16UC1 or 32FC1)
          cv_depth_ptr = cv_bridge::toCvShare(depth_msg, depth_msg->encoding);
        } catch (cv_bridge::Exception & e) {
          RCLCPP_ERROR(this->get_logger(),
                       "cv_bridge exception: %s", e.what());
          return;
        }
        const cv::Mat & depth_image = cv_depth_ptr->image;

        uint detection_count = detection_p ->detections.size();
        for (uint i = 0; i < detection_count; i++) {
            const vision_msgs::msg::ObjectHypothesisWithPose *results = detection_p->detections[i].results.data();
            int detected_id = std::stoi(results->hypothesis.class_id);
            // double score = results->hypothesis.score;

            if (detected_id == target_class_id_) {
                // Check if the object is detected
                bbox_center_x = detection_p->detections[i].bbox.center.position.x;
                bbox_center_y = detection_p->detections[i].bbox.center.position.y;
                RCLCPP_INFO(this->get_logger(),
                    "Target BBOX X=%0.2f, Y=%0.2f", bbox_center_x, bbox_center_y);

                int center_row = static_cast<int>(bbox_center_y);
                int center_col = static_cast<int>(bbox_center_x);
                const int half_w = 5;  // 5 + 5 = 10 pixels total
                float sum = 0.0f;
                int count = 0;

                // Loop over a 10×10 patch
                for (int dy = -half_w; dy < half_w; ++dy) {
                    int r = center_row + dy;
                    if (r < 0 || r >= depth_image.rows) continue;
                    for (int dx = -half_w; dx < half_w; ++dx) {
                        int c = center_col + dx;
                        if (c < 0 || c >= depth_image.cols) continue;
                        if (depth_image.type() == CV_16UC1) {
                            sum += depth_image.at<uint16_t>(r, c) * 0.001f;
                        }
                        else if (depth_image.type() == CV_32FC1) {
                            sum += depth_image.at<float>(r, c);
                        }
                        ++count;
                    }
                }

                if (count > 0) {
                    bbox_depth = sum / static_cast<float>(count);
                } else {
                    bbox_depth = 0.0f;
                }
                RCLCPP_INFO(this->get_logger(),
                    "Depth at (%.0f,%.0f) = %.3f m",
                    bbox_center_x, bbox_center_y, bbox_depth);

                object_detected = true;
                return;
            }
            else
                object_detected = false;
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
