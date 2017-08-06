// Developed by Wei Lisi, tlsirius@163.com
// July 26th,2017  

// main.cpp : Defines the entry point for the console application.
#include "tracking.h"
 
int SVMTraining = 0;	
//SVMTraining = 0 means testing; 
//SVMTraining = 1 means training new data.
const char *fileName = "test2.avi";
//Change fileName accordingly.

int main()
{
	Tracking objectTracking;
	CvSVM SVM1;

	/*int nReply = objectTracking.DisplayHelp();
	switch (nReply)
	{
	case 1:
		break;
	case 2:
		break;
	default:
		return 0;
	}
	*/



	//------------------------------Initiating and loading video----------------------------------------

	IplImage* pSrcFrame; // Source frame image
	IplImage* pGaussian; 
	Target target[10];	// Tracking targets
	Target tempTarget[10]; // Detected targets in current frame
	int targetNum = 0;  // Number of tracking targets 
	int tempTargetNum = 0; // Number of detected targets in current frame


	CvCapture* pSrcCapture;
	pSrcCapture = cvCaptureFromFile(fileName);
	if (pSrcCapture==NULL)
	{
		cout << "Error! "<<fileName<<" Not Found!" << endl;
		return 0;
	}
	pSrcCapture = cvCaptureFromFile(fileName); //load video


	pSrcFrame = cvQueryFrame(pSrcCapture);
	Mat mSrcFrame3;
	if (pSrcFrame->width > 800)  // If the video is too large, scale it to proper size
	{
		Mat tempMSrcFrame3(pSrcFrame);
		resize(tempMSrcFrame3, mSrcFrame3, Size(0, 0), 0.3, 0.3);
	}
	else
	{
		mSrcFrame3 = Mat(pSrcFrame);
	}

	CvVideoWriter* pWriterTracking = cvCreateVideoWriter("tracking.wmv", CV_FOURCC('W', 'M', 'V', '1'), 25,cvSize(mSrcFrame3.size().width, 2*mSrcFrame3.size().height) ,1);
	//tracking.avi: result video of tracking and wake detection

	Mat prevgray; //Optical flow result


	
	//--------------------------------------Loop start----------------------------------------


	for (int frame = 0; ; frame++)
	{
		//-----------------------------------------Image Processing--------------------------------------

		pSrcFrame = cvQueryFrame(pSrcCapture);
		pSrcFrame = cvLoadImage("0001.jpg");
		if (pSrcFrame == NULL)
		{
			cvReleaseVideoWriter(&pWriterTracking);
			return 0;
		}
		pGaussian = cvCreateImage(cvGetSize(pSrcFrame), 8, 1);
		Mat mSrcFrame3;
		if (pSrcFrame->width > 800)// If the video is too large, scale it to proper size
		{
			Mat tempMSrcFrame3(pSrcFrame);
			resize(tempMSrcFrame3, mSrcFrame3, Size(0, 0), 0.3, 0.3);
		}
		else
		{
			mSrcFrame3 = Mat(pSrcFrame);
		}

		Mat mSrcFrame;
		cvtColor(mSrcFrame3, mSrcFrame, CV_RGB2GRAY); //Convert 3 channel RGB image to Gray image

		Mat mGaussian;
		GaussianBlur(mSrcFrame, mGaussian, cvSize(9, 9), 0);  //Gaussian blur
		for (int i = 0; i < 5; i++)
		{
			GaussianBlur(mGaussian, mGaussian, cvSize(9, 9), 0);
		}

		Mat mGroundSkyTest;
		threshold(mGaussian, mGroundSkyTest, 0, 255, THRESH_OTSU); 
		//Otsu thresholding the Gaussian blured image
		//Get the raw image for horizon detecion
	


		Mat mDiff;

		absdiff(mSrcFrame, mGaussian, mDiff); //absolute value of the difference between original image and Gaussian blured image
		//imshow("Diff", mDiff);
		Mat mDiffGaussian;

		GaussianBlur(mDiff, mDiffGaussian, cvSize(9, 9), 0); //Gaussian blur
		for (int i = 0; i < 1; i++)
		{
			GaussianBlur(mDiffGaussian, mDiffGaussian, cvSize(9, 9), 0);
		}
		Mat mOtsuThreshold;
		threshold(mDiffGaussian, mOtsuThreshold, 30, 255, THRESH_BINARY);
		// Global thresholding


		//-------------------------------------- Displaying the results------------------------------------------

		imshow("Origin Image", mSrcFrame);
		imshow("Gaussian", mGaussian);
		imshow("Diff", mDiff);
		imshow("DiffGaussian", mDiffGaussian);
		imshow("otsu threshold", mOtsuThreshold);
		imshow("horizon-line", mGroundSkyTest);
		cvWaitKey(0);

		
	}

	cvReleaseVideoWriter(&pWriterTracking);
	
	return 0;
}

