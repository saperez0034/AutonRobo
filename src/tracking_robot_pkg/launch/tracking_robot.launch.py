from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='tracking_robot_pkg',
            executable='tracking_robot_client',
            output='screen',
            emulate_tty=True)
    ])