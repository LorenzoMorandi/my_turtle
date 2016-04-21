#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "turtlesim/Pose.h"
#include "my_turtle/RobotStatus.h"

ros::Subscriber team_status_sub;
ros::Publisher team_status_pub;

std::map<std::string,bool> robots_state;
int ROBOT_NUMBER=3;
std::string robot_name;

turtlesim::Pose position;

bool teamReady = false;

void poseCallback(const turtlesim::PoseConstPtr& msg)
{
// 	ROS_INFO("x: %f y: %f", msg->x, msg->y);
	position = *msg;
}	

void publishReadyStatus() { 
  my_turtle::RobotStatus status_msg;

    status_msg.header.stamp = ros::Time::now();
    status_msg.robot_id = robot_name;
    status_msg.is_ready = true;

    // Wait for the publisher to connect to subscribers
    sleep(1.0);
    team_status_pub.publish(status_msg);

    ROS_INFO_STREAM("Robot published ready status" << robot_name);
  }
  
void waitForTeam()  {
    ros::Rate loopRate(1);
    // Wait until all robots are ready...
    while (!teamReady) {
        ROS_INFO_STREAM("Robot waiting for team" <<robot_name);
        publishReadyStatus();
        ros::spinOnce();
        loopRate.sleep();
    }
}


void teamStatusCallback(const my_turtle::RobotStatus::ConstPtr& status_msg)
{
	// Check if message came from monitor
 
	if(teamReady) return;
  
	robots_state[status_msg->robot_id] = status_msg->is_ready;
	int ready_counter = 0;
	for(auto robot : robots_state)
	{
	  if(robot.second) ready_counter++;
	  }
	if(ready_counter==ROBOT_NUMBER){
	ROS_INFO_STREAM("Robot: Team is ready. Let's move!" <<robot_name);
	teamReady = true;
	}
}


int main(int argc, char **argv)
{
	const double FORWARD_SPEED_MPS=0.5;
	const double ANGULAR_SPEED_MPS=0.3;
  
	ros::init(argc, argv, "move_turtle");
	if (argc < 2) {
	  ROS_ERROR("You must specify robot id.");
	  return -1;
	}
	
	robot_name = std::string(argv[1]);
	
	ros::NodeHandle node;
	ros::Publisher pub = node.advertise<geometry_msgs::Twist>(robot_name + "/cmd_vel",10);
	
	team_status_pub = node.advertise<my_turtle::RobotStatus>("team_status",10);
	team_status_sub = node.subscribe("team_status", 20, &teamStatusCallback);
	
	publishReadyStatus();
	
	waitForTeam();
	
		
	
	ros::Subscriber sub=node.subscribe("turtle1/pose", 10, poseCallback);
	
	geometry_msgs::Twist msg;
	
	msg.linear.x=FORWARD_SPEED_MPS;
	msg.angular.z=ANGULAR_SPEED_MPS;
	
	ros::Rate loop_rate(10);

	ROS_INFO("starting to move forward");
	
	int count=0;
	while (ros::ok())
	{
		pub.publish(msg);
		ros::spinOnce();
		loop_rate.sleep();
	}
	return 0;
}
