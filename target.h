// Developed by Wei Lisi, tlsirius@163.com
// April 26th,2017  


#pragma once

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <math.h>

#include <fstream>
using namespace cv;
using namespace std;

#define UNKNOWN_FLOW_THRESH 1e9  


class Target //Class for the tracking targets
{
public:
	bool sameTarget(Target); // Estimate if the given target is the same target as this one.	
	int width; //Width of the bounding box of the target.
	int height; //Height of the bounding box of the target.
	int x;	//The x-coordinate of the top-left point of the bounding box of the target
	int y;	//The y-coordinate of the top-left point of the bounding box of the target
	int LastTrackingFrame;  //The last frame that this target is tracked.

public:
	Target();
	Target(const Target& target);
	~Target();
	Point center() { return Point((x + width / 2), (y + height / 2)); }; //Return the center position of the target
	KalmanFilter* kalman;	//Kalman filter of the center position of the target bounding box.
};

