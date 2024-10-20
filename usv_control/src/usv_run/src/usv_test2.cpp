#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Float32.h>

// 全局变量：船坞的水平位置（假设在图像中的x坐标）
float dock_position_x = -1;  // -1 表示未检测到船坞

// 回调函数，获取摄像头识别的船坞位置
void dockPositionCallback(const std_msgs::Float32::ConstPtr& msg) {
    dock_position_x = msg->data;  // 获取船坞的x坐标
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "simple_boat_docking");
    ros::NodeHandle nh;

    // 订阅船坞位置（由摄像头识别模块发布）
    ros::Subscriber dock_pos_sub = nh.subscribe("/camera/dock_position_x", 10, dockPositionCallback);

    // 创建发布速度控制指令的发布者
    ros::Publisher cmd_vel_pub = nh.advertise<geometry_msgs::Twist>("/mavros/setpoint_velocity/cmd_vel_unstamped", 10);

    // ROS 循环频率
    ros::Rate rate(10);  // 10Hz

    // 设置船坞在图像中的目标位置（假设图像宽度为640像素，320为图像中心）
    const float dock_center_x = 320;
    const float max_speed = 0.5;      // 最大前进速度
    const float max_turn_speed = 0.3; // 最大旋转速度

    while (ros::ok()) {
        ros::spinOnce();  // 更新回调函数数据

        geometry_msgs::Twist cmd_vel_msg;

        // 如果检测到船坞
        if (dock_position_x >= 0) {
            // 计算船坞相对于图像中心的偏移量
            float error_x = dock_position_x - dock_center_x;

            // 根据偏移量调整无人艇的旋转
            cmd_vel_msg.angular.z = -0.002 * error_x;  // 旋转速度与偏差成比例

            // 无人艇一直前进
            cmd_vel_msg.linear.x = max_speed;

            ROS_INFO("Dock detected. Adjusting course. Error: %f", error_x);

        } else {
            // 如果未检测到船坞，停止前进和转向
            cmd_vel_msg.linear.x = 0.0;
            cmd_vel_msg.angular.z = 0.0;
            ROS_WARN("No dock detected. Waiting...");
        }

        // 发布速度指令
        cmd_vel_pub.publish(cmd_vel_msg);

        rate.sleep();
    }

    return 0;
}
