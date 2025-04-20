from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package= 'ros_ign_bridge',
            executable='parameter_bridge',
            arguments=['/cmd_vel@geometry_msgs/msg/Twist@ignition.msgs.Twist']),
        Node(
            package= 'ros_ign_bridge',
            executable='parameter_bridge',
            arguments=['/lidar@sensor_msgs/msg/LaserScan@ignition.msgs.LaserScan']),
        Node(
            package='topic_publisher_pkg',
            executable='circle_wall',
            output='screen'),
    ])
