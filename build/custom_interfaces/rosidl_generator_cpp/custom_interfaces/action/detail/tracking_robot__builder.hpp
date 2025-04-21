// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from custom_interfaces:action/TrackingRobot.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACES__ACTION__DETAIL__TRACKING_ROBOT__BUILDER_HPP_
#define CUSTOM_INTERFACES__ACTION__DETAIL__TRACKING_ROBOT__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "custom_interfaces/action/detail/tracking_robot__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace custom_interfaces
{

namespace action
{

namespace builder
{

class Init_TrackingRobot_Goal_object
{
public:
  Init_TrackingRobot_Goal_object()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interfaces::action::TrackingRobot_Goal object(::custom_interfaces::action::TrackingRobot_Goal::_object_type arg)
  {
    msg_.object = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_Goal msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::action::TrackingRobot_Goal>()
{
  return custom_interfaces::action::builder::Init_TrackingRobot_Goal_object();
}

}  // namespace custom_interfaces


namespace custom_interfaces
{

namespace action
{

namespace builder
{

class Init_TrackingRobot_Result_result
{
public:
  Init_TrackingRobot_Result_result()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interfaces::action::TrackingRobot_Result result(::custom_interfaces::action::TrackingRobot_Result::_result_type arg)
  {
    msg_.result = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_Result msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::action::TrackingRobot_Result>()
{
  return custom_interfaces::action::builder::Init_TrackingRobot_Result_result();
}

}  // namespace custom_interfaces


namespace custom_interfaces
{

namespace action
{

namespace builder
{

class Init_TrackingRobot_Feedback_feedback
{
public:
  Init_TrackingRobot_Feedback_feedback()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interfaces::action::TrackingRobot_Feedback feedback(::custom_interfaces::action::TrackingRobot_Feedback::_feedback_type arg)
  {
    msg_.feedback = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_Feedback msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::action::TrackingRobot_Feedback>()
{
  return custom_interfaces::action::builder::Init_TrackingRobot_Feedback_feedback();
}

}  // namespace custom_interfaces


namespace custom_interfaces
{

namespace action
{

namespace builder
{

class Init_TrackingRobot_SendGoal_Request_goal
{
public:
  explicit Init_TrackingRobot_SendGoal_Request_goal(::custom_interfaces::action::TrackingRobot_SendGoal_Request & msg)
  : msg_(msg)
  {}
  ::custom_interfaces::action::TrackingRobot_SendGoal_Request goal(::custom_interfaces::action::TrackingRobot_SendGoal_Request::_goal_type arg)
  {
    msg_.goal = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_SendGoal_Request msg_;
};

class Init_TrackingRobot_SendGoal_Request_goal_id
{
public:
  Init_TrackingRobot_SendGoal_Request_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_TrackingRobot_SendGoal_Request_goal goal_id(::custom_interfaces::action::TrackingRobot_SendGoal_Request::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return Init_TrackingRobot_SendGoal_Request_goal(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_SendGoal_Request msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::action::TrackingRobot_SendGoal_Request>()
{
  return custom_interfaces::action::builder::Init_TrackingRobot_SendGoal_Request_goal_id();
}

}  // namespace custom_interfaces


namespace custom_interfaces
{

namespace action
{

namespace builder
{

class Init_TrackingRobot_SendGoal_Response_stamp
{
public:
  explicit Init_TrackingRobot_SendGoal_Response_stamp(::custom_interfaces::action::TrackingRobot_SendGoal_Response & msg)
  : msg_(msg)
  {}
  ::custom_interfaces::action::TrackingRobot_SendGoal_Response stamp(::custom_interfaces::action::TrackingRobot_SendGoal_Response::_stamp_type arg)
  {
    msg_.stamp = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_SendGoal_Response msg_;
};

class Init_TrackingRobot_SendGoal_Response_accepted
{
public:
  Init_TrackingRobot_SendGoal_Response_accepted()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_TrackingRobot_SendGoal_Response_stamp accepted(::custom_interfaces::action::TrackingRobot_SendGoal_Response::_accepted_type arg)
  {
    msg_.accepted = std::move(arg);
    return Init_TrackingRobot_SendGoal_Response_stamp(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_SendGoal_Response msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::action::TrackingRobot_SendGoal_Response>()
{
  return custom_interfaces::action::builder::Init_TrackingRobot_SendGoal_Response_accepted();
}

}  // namespace custom_interfaces


namespace custom_interfaces
{

namespace action
{

namespace builder
{

class Init_TrackingRobot_GetResult_Request_goal_id
{
public:
  Init_TrackingRobot_GetResult_Request_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interfaces::action::TrackingRobot_GetResult_Request goal_id(::custom_interfaces::action::TrackingRobot_GetResult_Request::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_GetResult_Request msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::action::TrackingRobot_GetResult_Request>()
{
  return custom_interfaces::action::builder::Init_TrackingRobot_GetResult_Request_goal_id();
}

}  // namespace custom_interfaces


namespace custom_interfaces
{

namespace action
{

namespace builder
{

class Init_TrackingRobot_GetResult_Response_result
{
public:
  explicit Init_TrackingRobot_GetResult_Response_result(::custom_interfaces::action::TrackingRobot_GetResult_Response & msg)
  : msg_(msg)
  {}
  ::custom_interfaces::action::TrackingRobot_GetResult_Response result(::custom_interfaces::action::TrackingRobot_GetResult_Response::_result_type arg)
  {
    msg_.result = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_GetResult_Response msg_;
};

class Init_TrackingRobot_GetResult_Response_status
{
public:
  Init_TrackingRobot_GetResult_Response_status()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_TrackingRobot_GetResult_Response_result status(::custom_interfaces::action::TrackingRobot_GetResult_Response::_status_type arg)
  {
    msg_.status = std::move(arg);
    return Init_TrackingRobot_GetResult_Response_result(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_GetResult_Response msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::action::TrackingRobot_GetResult_Response>()
{
  return custom_interfaces::action::builder::Init_TrackingRobot_GetResult_Response_status();
}

}  // namespace custom_interfaces


namespace custom_interfaces
{

namespace action
{

namespace builder
{

class Init_TrackingRobot_FeedbackMessage_feedback
{
public:
  explicit Init_TrackingRobot_FeedbackMessage_feedback(::custom_interfaces::action::TrackingRobot_FeedbackMessage & msg)
  : msg_(msg)
  {}
  ::custom_interfaces::action::TrackingRobot_FeedbackMessage feedback(::custom_interfaces::action::TrackingRobot_FeedbackMessage::_feedback_type arg)
  {
    msg_.feedback = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_FeedbackMessage msg_;
};

class Init_TrackingRobot_FeedbackMessage_goal_id
{
public:
  Init_TrackingRobot_FeedbackMessage_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_TrackingRobot_FeedbackMessage_feedback goal_id(::custom_interfaces::action::TrackingRobot_FeedbackMessage::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return Init_TrackingRobot_FeedbackMessage_feedback(msg_);
  }

private:
  ::custom_interfaces::action::TrackingRobot_FeedbackMessage msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::action::TrackingRobot_FeedbackMessage>()
{
  return custom_interfaces::action::builder::Init_TrackingRobot_FeedbackMessage_goal_id();
}

}  // namespace custom_interfaces

#endif  // CUSTOM_INTERFACES__ACTION__DETAIL__TRACKING_ROBOT__BUILDER_HPP_
