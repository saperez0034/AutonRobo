from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package= 'ros_gz_bridge',
            executable='parameter_bridge',
            arguments=['/cmd_vel@geometry_msgs/msg/Twist@ignition.msgs.Twist']),
        Node(
            package= 'ros_ign_bridge',
            executable='parameter_bridge',
            arguments=['/lidar@sensor_msgs/msg/LaserScan@ignition.msgs.LaserScan']),
        Node(
            package= 'ros_gz_bridge',
            executable='parameter_bridge',
            arguments=['/wall/touched@std_msgs/msg/Bool@ignition.msgs.Boolean']),
        Node(
            package='circle_wall_actions_pkg',
            executable='circle_wall_client',
            output='screen'),
        Node(
            package='circle_wall_actions_pkg',
            executable='circle_wall_server',
            output='screen'),
    ])
