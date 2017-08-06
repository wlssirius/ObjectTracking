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
	Kalman = cvCreateKalman(4, 2, 0);
	const CvMat* prediction = cvKalmanPredict(Kalman, 0);
	//cout<<"predict"<<prediction->data.fl[0]<<" "<<prediction->data.fl[1]<<endl;
	CvMat* process_noise = cvCreateMat(4, 1, CV_32FC1);
	CvRNG rng = cvRNG(-1);

	float a[4][4] = {//transition matrix
		1,0,1,0,
		0,1,0,1,
		0,0,1,0,
		0,0,0,1
	};
	memcpy(Kalman->transition_matrix->data.fl, a, sizeof(a));
	cvSetIdentity(Kalman->measurement_matrix, cvRealScalar(1));
	cvSetIdentity(Kalman->process_noise_cov, cvRealScalar(1e-5));
	cvSetIdentity(Kalman->measurement_noise_cov, cvRealScalar(1e-1));
	cvSetIdentity(Kalman->error_cov_post, cvRealScalar(1));
	cvReleaseMat(&process_noise);
}

bool Target::sameTarget(Target newTarget)
{	// Estimate if the given target is the same target as this one.	
	int area = width*height;
	int newArea = newTarget.width*newTarget.height;
	if (5 * area < newArea || area>5 * newArea)
		return false;
	const CvMat* prediction;
	prediction = cvKalmanPredict(Kalman, 0);
	Point predictCenter(prediction->data.fl[0], prediction->data.fl[1]);
	if (predictCenter.x > newTarget.x&&predictCenter.x < (newTarget.x + newTarget.width) && predictCenter.y>newTarget.y&&predictCenter.y < (newTarget.y + newTarget.height))
	{
		return true;
	}
	else if (newTarget.center().x > x&&newTarget.center().x < (x + width) && newTarget.center().y>y&&newTarget.center().y < (y + height))
	{
		return true;
	}
	else return false;
}


