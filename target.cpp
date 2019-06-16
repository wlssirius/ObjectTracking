// Developed by Wei Lisi, tlsirius@163.com
// July 26th,2017  

#include "target.h"

Target::Target()//Class for the tracking targets
{
	width = -1;
	height = -1;
	x = -1;
	y = -1;
	LastTrackingFrame = -1;
	kalman = new KalmanFilter(4, 2, 0); 
	Mat state(4, 1, CV_32F); /* (phi, delta_phi) */
	Mat processNoise(4, 1, CV_32F);
	Mat measurement = Mat::zeros(2, 1, CV_32F);
	char code = (char)-1;

	randn(state, Scalar::all(0), Scalar::all(0.1));
	kalman->transitionMatrix = (Mat_<float>(4, 4) <<
		1, 0, 1, 0,
		0, 1, 0, 1,
		0, 0, 1, 0,
		0, 0, 0, 1);
	setIdentity(kalman->measurementMatrix);
	setIdentity(kalman->processNoiseCov, Scalar::all(1e-5));
	setIdentity(kalman->measurementNoiseCov, Scalar::all(1e-1));
	setIdentity(kalman->errorCovPost, Scalar::all(1));

	randn(kalman->statePost, Scalar::all(0), Scalar::all(0.1));
}

Target::Target(const Target & target)
{
	x = target.x;
	y = target.y;
	width = target.width;
	height = target.height;
	LastTrackingFrame = target.LastTrackingFrame;
	kalman = target.kalman;
}

Target::~Target()
{
}

bool Target::sameTarget(Target newTarget)
{	// Estimate if the given target is the same target as this one.	
	int area = width*height;
	int newArea = newTarget.width*newTarget.height;
	if (5 * area < newArea || area>5 * newArea)
		return false;
	Mat prediction = kalman->predict();
	Point predictCenter(prediction.at<float>(0), prediction.at<float>(1));
	if (predictCenter.x > newTarget.x&&predictCenter.x < (newTarget.x + newTarget.width) && predictCenter.y>newTarget.y&&predictCenter.y < (newTarget.y + newTarget.height))
	{
		return true;
	}
	else if (newTarget.center().x > x&&newTarget.center().x < (x + width) && newTarget.center().y>y&&newTarget.center().y < (y + height))
	{
		return true;
	}
	
	return false;
}


