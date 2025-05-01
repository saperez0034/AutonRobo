// serial_twist_bridge.cpp
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

class SerialTwistBridge : public rclcpp::Node
{
public:
  SerialTwistBridge()
  : Node("serial_twist_bridge")
  {
    // Declare & get parameters
    this->declare_parameter<std::string>("serial_port", "/dev/ttyACM0");
    this->declare_parameter<int>("baud_rate", 115200);
    this->declare_parameter<std::string>("twist_topic", "cmd_vel");
    this->declare_parameter<int>("T_value", 13);

    port_name_ = this->get_parameter("serial_port").as_string();
    baud_rate_ = this->get_parameter("baud_rate").as_int();
    twist_topic_ = this->get_parameter("twist_topic").as_string();
    T_ = this->get_parameter("T_value").as_int();

    // Open & configure serial port
    fd_ = open(port_name_.c_str(), O_RDWR | O_NOCTTY);
    if (fd_ < 0) {
      RCLCPP_FATAL(this->get_logger(), "Failed to open %s: %s",
                   port_name_.c_str(), strerror(errno));
      rclcpp::shutdown();
      return;
    }
    configurePort();

    // Subscribe to Twist
    sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
      twist_topic_, 10,
      std::bind(&SerialTwistBridge::twistCallback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(),
                "Bridging '%s' → %s at %d baud",
                twist_topic_.c_str(), port_name_.c_str(), baud_rate_);
  }

  ~SerialTwistBridge()
  {
    if (fd_ >= 0) {
      close(fd_);
    }
  }

private:
  void configurePort()
  {
    termios tty;
    if (tcgetattr(fd_, &tty) != 0) {
      RCLCPP_FATAL(this->get_logger(), "tcgetattr: %s", strerror(errno));
      return;
    }

    // Set baud
    speed_t speed = B115200;
    switch (baud_rate_) {
      case 9600:   speed = B9600;  break;
      case 19200:  speed = B19200; break;
      case 38400:  speed = B38400; break;
      case 57600:  speed = B57600; break;
      case 115200: speed = B115200;break;
      default:
        RCLCPP_WARN(this->get_logger(),
                    "Unsupported baud %d, defaulting to 115200",
                    baud_rate_);
    }
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    // 8N1, no flow control, raw mode
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_cflag &= ~(PARENB | PARODD | CSTOPB | CRTSCTS);

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_lflag = 0;
    tty.c_oflag = 0;

    tty.c_cc[VMIN]  = 1;    // blocking read until 1 byte arrives
    tty.c_cc[VTIME] = 1;    // 0.1s read timeout

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
      RCLCPP_FATAL(this->get_logger(), "tcsetattr: %s", strerror(errno));
    }
  }

  void twistCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    // Build JSON: {"T":<T>,"X":<linear.x>,"Z":<angular.z>}
    std::ostringstream ss;
    ss << "{\"T\":" << T_
       << ",\"X\":" << msg->linear.x
       << ",\"Z\":" << msg->angular.z
       << "}\n";
    std::string json = ss.str();

    ssize_t written = write(fd_, json.c_str(), json.size());
    if (written < 0) {
      RCLCPP_ERROR(this->get_logger(),
                   "Serial write failed: %s", strerror(errno));
    }
    else {
        RCLCPP_INFO(this->get_logger(),
                     "Wrote %zd bytes: %s", written, json.c_str());
    }
    sleep(0.5); // Allow time for the command to be processed
  }

  // Node state
  std::string port_name_, twist_topic_;
  int baud_rate_, T_;
  int fd_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<SerialTwistBridge>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
