#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
	    //ROS_INFO("Driving the robot towards the ball l_x: %1.2f, a_z: %1.2f", lin_x, ang_z);

	    ball_chaser::DriveToTarget srv;
	    srv.request.linear_x =lin_x;
	    srv.request.angular_z = ang_z;

	    // Call the safe_move service and pass the requested joint angles
	    if (!client.call(srv))
				ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 240;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
		//ROS_INFO("Processing image");
		
		float angle = 0;
		for (int r = 0; r < img.height; r++) {
			int first = -1, last = -1;
			int row_offset = r * img.step;
			// We're looking for white so let's take one channel only
			for (int c = 0; c < img.step; c += 3) {
				if (img.data[row_offset + c] > white_pixel) {
					if (first == -1) first = c;
					last = c;				
				}
			}
			// found some
			if (first != -1) {
				float ball_center = (first + last)/2;
				// empirical factor for angle (0.05) given my poor system perfomances (macos + vm)
				// also, x velocity is zero on purpose
				angle = 0.05*(1 - ball_center/(img.step/2)); 
		
				drive_robot(0, angle);

				return;
			}
		}
		// not found, stop
		drive_robot(0, 0);
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
