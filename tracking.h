// Developed by Wei Lisi, tlsirius@163.com
// April 26th,2017  


#pragma once

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <math.h>
#include <vector>
#include "target.h"

#include <fstream>
using namespace cv;
using namespace std;

#define MIN_TARGET_SIZE	250
#define MAX_TARGET_NUM 10

#define REPLY_OK 0
#define REPLY_ERR 1

#define DISPLAY_RESULT true;

#define FILE_NAME "..\\img\\%04d.jpg"

class Tracking //Class for the tracking procedure
{
private:
	vector<Target> target;	// Tracking targets
	vector<Target> tempTarget; // Detected targets in current frame
	VideoCapture m_pVideoCapture;
	VideoWriter* m_pWriterTracking;
	Mat m_mSrcFrame3;
	Mat m_mSrcFrame;
	Mat m_mDiffGaussian;
	Mat m_mThreshold;
	int nFrame;
public:
	Tracking();
	~Tracking() ;
	int DisplayResult(int nTime);
	int GetFrameNum();

	int CreateResultVideo();
	int WriteResultVideo();
	int ReleaseResultVideo();

	int LoadSequence(char* pcFileName);
	int LoadNextFrame();

	int ProcessImage();
	int DetectTarget();
	int TrackTarget();
};


Point findCenter(Mat Image, vector<Point> contour);
