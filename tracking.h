// Developed by Wei Lisi, tlsirius@163.com
// April 26th,2017  


#pragma once

#include "cv.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <math.h>
#include "target.h"

#include <fstream>
using namespace cv;
using namespace std;

#define UNKNOWN_FLOW_THRESH 1e9  


class Tracking //Class for the tracking procedure
{
private:
	Target m_target;
public:
	Tracking();
	~Tracking() {};
	int DisplayHelp();
	int LoadSequence();
	int TrainObject(CvSVM *svm);
	/*
	void makecolorwheel(vector<Scalar> &colorwheel); // Convert the vector to corrsponding color.
	void motionToColor(Mat flow, Mat &color); // Plot optical flow result as colored image.
	double gaussrand(); // Generate random number with Gaussian distribution
	void calcIntensityHist(Mat flowBlock, int *hist0); // Calculate the histogram of intensity in the given block.
	void train(Mat flow, Mat origin, Rect r, char *fileName); // Train the wake feature using SVM.
	void predict(Mat flow, Mat origin, Rect r, CvSVM* SVM, Mat sub); // Detect the wakes in the given region using SVM.
	CvPoint findCenter(const IplImage * Image, CvSeq* contour); // Find the center point of the given connected domain.
	CvRect generateGroundRegion(Mat mGroundSky); // Generate the ground region
	*/
};
