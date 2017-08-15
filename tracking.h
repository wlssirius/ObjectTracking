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

#define MIN_TARGET_SIZE	250
#define MAX_TARGET_NUM 10

#define REPLY_OK 0
#define REPLY_ERR 1

#define DISPLAY_RESULT true;

#define FILE_NAME "img\\%04d.jpg"

class Tracking //Class for the tracking procedure
{
private:
	Target target[10];	// Tracking targets
	Target tempTarget[10]; // Detected targets in current frame
	int targetNum ;  // Number of tracking targets 
	int tempTargetNum ; // Number of detected targets in current frame
	CvCapture* m_pSrcCapture; 
	CvVideoWriter* m_pWriterTracking;
	Mat m_mSrcFrame3;
	Mat m_mSrcFrame;
	Mat m_mDiffGaussian;
	Mat m_mThreshold;
	int nFrame;
public:
	Tracking();
	~Tracking() ;
	int DisplayHelp();
	int DisplayResult(int nTime);
	int GetFrameNum();

	int CreateResultVideo();
	int WriteResultVideo();
	int ReleaseResultVideo();

	int LoadSequence(char* pcFileName);
	int LoadNextFrame();

	int TrainObject(CvSVM *svm);
	int ProcessImage();
	int DetectTarget();
	int TrackTarget();
};


CvPoint findCenter(const IplImage * Image, CvSeq* contour);
