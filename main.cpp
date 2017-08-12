// Developed by Wei Lisi, tlsirius@163.com
// July 26th,2017  

// main.cpp : Defines the entry point for the console application.
#include "tracking.h"

#define MIN_AREA	200

const char *fileName = "%04d.jpg";
//Change fileName accordingly.

int main()
{
	
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
	//tracking.wmv: result video of tracking and wake detection

	
	//--------------------------------------Loop start----------------------------------------


	for (int frame = 0; ; frame++)
	{
		//-----------------------------------------Image Processing--------------------------------------

		pSrcFrame = cvQueryFrame(pSrcCapture);
		//pSrcFrame = cvLoadImage("Car24\\img\\%04d.jpg");
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
		cvWaitKey(1);

		//----------------------------Removing small area---------------------------------
		Mat mSub, mSubOtsuThreshold; 
		IplImage pTest = IplImage(mGroundSkyTest);
		IplImage *pThreshold = cvCreateImage(cvGetSize(&pTest), 8, 1);
		IplImage *pThresholdOpt = cvCreateImage(cvGetSize(&pTest), 8, 1);
		IplImage pTemp = IplImage(mOtsuThreshold);
		pThreshold = cvCloneImage(&pTemp);
		cvZero(pThreshold);
		cvZero(pThresholdOpt);
		CvMemStorage* memStorage;
		memStorage = cvCreateMemStorage(0);
		CvSeq *firstContour;
		cvFindContours(&pTemp, memStorage, &firstContour, sizeof(CvContour),
			CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);    // Search for connected domains in the result of otsu thresholding
		CvSeq*contour2;
		contour2 = firstContour;
		int it = 0;
		for (; contour2 != 0; contour2 = contour2->h_next)
		{
			double tmparea = fabs(cvContourArea(contour2));
			it++;
			if (tmparea > MIN_AREA)	//If the area of the connected domain is smaller than 500, it is removed. 
			{


				//----------------------Local thresholding-------------------------
				CvRect r = ((CvContour*)contour2)->rect;
				cvSetImageROI(pSrcFrame, r);
				CvScalar avg = cvAvg(pSrcFrame);
				mSub = mDiffGaussian(Range(r.y, r.y + r.height), Range(r.x, r.x + r.width));
				threshold(mSub, mSubOtsuThreshold, 20, 255, THRESH_OTSU); //Otsu thresholding at the local area around the target.
				Mat kernel;
																		  //Morphological filtering to remove small areas.
				for (int j = 0; j < 3; j++)
				{
					erode(mSubOtsuThreshold, mSubOtsuThreshold, kernel);
				}
				for (int j = 0; j < 5; j++)
				{
					dilate(mSubOtsuThreshold, mSubOtsuThreshold, kernel);
				}

				cvDrawContours(pThreshold, contour2, CV_RGB(0, 255, 255), CV_RGB(0, 255, 255), -1, -1, 8);

				cvSetImageROI(pThresholdOpt, r);

				IplImage pSubTemp = IplImage(mSubOtsuThreshold);
				cvCopy(&pSubTemp, pThresholdOpt, 0);	//Copy the local thresholding image back to the entire image
				int mWidth = (mGaussian.size()).width;
				int mHeight = (mGaussian.size()).height;
				cvSetImageROI(pThresholdOpt, cvRect(0, 0, mWidth, mHeight));
				cvShowImage("threshold", pThresholdOpt);
				cvWaitKey(10);
				
				
				//--------------------------------Extracting target shape from local thresholding result----------------------
				CvMemStorage* memStorage;
				memStorage = cvCreateMemStorage(0);
				CvSeq *firstContour;
				CvSeq*contourColor;
				//IplImage pErode = IplImage(pThreshold);

				cvFindContours(pThresholdOpt, memStorage, &firstContour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
				// Search for connected domains in the result of local otsu thresholding
				contourColor = firstContour;
				for (; contourColor != 0; contourColor = contourColor->h_next)
				{
					double tmparea = fabs(cvContourArea(contourColor));

					CvRect r = ((CvContour*)contourColor)->rect;

					if (tmparea < MIN_AREA) //If the area of the connected domain is smaller than dynamicArea, it is removed.  
					{
						continue;
					}
					CvPoint center = findCenter(pThresholdOpt, contourColor);
					int x = int(center.x + r.x);
					int y = int(center.y + r.y);

					if (tempTargetNum < 10) // Save the detected target in tempTarget[]. 
					{
						tempTarget[tempTargetNum].x = r.x;
						tempTarget[tempTargetNum].y = r.y;
						tempTarget[tempTargetNum].width = r.width;
						tempTarget[tempTargetNum].height = r.height;
						tempTarget[tempTargetNum].LastTrackingFrame = frame;
						tempTargetNum++;
					}
					
				}
			}

		}

		//--------------------------------Tracking targets by Kalman filtering-----------------------------

		if (targetNum == 0) //If No targets have been tracked, then track all the targets in current frame
		{
			for (int i = 0; i < tempTargetNum; i++)
			{
				target[i].x = tempTarget[i].x;
				target[i].y = tempTarget[i].y;
				target[i].width = tempTarget[i].width;
				target[i].height = tempTarget[i].height;
				target[i].LastTrackingFrame = frame;

				//Initializing the Kalman filter.
				CvMat* measurement = cvCreateMat(2, 1, CV_32FC1);
				const CvMat* prediction;
				measurement->data.fl[0] = float(target[i].x + target[i].width / 2 - 1);
				measurement->data.fl[1] = float(target[i].y + target[i].height / 2 - 1);
				for (int k = 0; k<10; k++)
				{
					cvKalmanCorrect(target[i].Kalman, measurement);
					prediction = cvKalmanPredict(target[i].Kalman, 0);
				}
				cvReleaseMat(&measurement);
				targetNum++;
			}
		}
		else // If some targets have been tracked in the last frame, try to match (based on position and size) these targets with the newly detected targets.
		{
			for (int i = 0; i < tempTargetNum; i++)
			{
				int trackFlag = 0;
				for (int j = 0; j < targetNum; j++)
				{
					if (target[j].sameTarget(tempTarget[i])) //estimate if the targets being tracked match with the newly detected targets
					{
						if (abs(target[j].center().x - tempTarget[i].center().x) < 10 && abs(target[j].center().y - tempTarget[i].center().y) < 10 && abs(target[j].width - tempTarget[i].width) < 10 && abs(target[j].height - tempTarget[j].height) < 10)
						{
							//if matched, update the state of the target.
							target[j].x = tempTarget[i].x;
							target[j].y = tempTarget[i].y;
							target[j].width = tempTarget[i].width;
							target[j].height = tempTarget[i].height;
							CvMat* measurement = cvCreateMat(2, 1, CV_32FC1);
							const CvMat* prediction;
							measurement->data.fl[0] = float(target[j].x + target[j].width / 2 - 1);
							measurement->data.fl[1] = float(target[j].y + target[j].height / 2 - 1);
							cvKalmanCorrect(target[j].Kalman, measurement);
							prediction = cvKalmanPredict(target[j].Kalman, 0);
							target[j].LastTrackingFrame = frame;
						}
						trackFlag = 1;

					}
				}
				if (trackFlag == 0)//If no match is found, save it as a new tracking target.
				{
					target[targetNum].x = tempTarget[i].x;
					target[targetNum].y = tempTarget[i].y;
					target[targetNum].width = tempTarget[i].width;
					target[targetNum].height = tempTarget[i].height;
					CvMat* measurement = cvCreateMat(2, 1, CV_32FC1);
					const CvMat* prediction;
					measurement->data.fl[0] = float(target[targetNum].x + target[targetNum].width / 2 - 1);
					measurement->data.fl[1] = float(target[targetNum].y + target[targetNum].height / 2 - 1);
					for (int k = 0; k<10; k++)
					{
						cvKalmanCorrect(target[targetNum].Kalman, measurement);
						prediction = cvKalmanPredict(target[targetNum].Kalman, 0);
						//cout << "prediction:" << prediction->data.fl[0] << " " << prediction->data.fl[1] << endl;
					}
					target[targetNum].LastTrackingFrame = frame;
					targetNum++;
				}
			}
		}

		for (int i = 0; i < targetNum; i++)
		{
			CvRect r;
			r.width = target[i].width;
			r.height = target[i].height;
			r.x = target[i].x;
			r.y = target[i].y;

			Scalar red(0, 0, 200);
			rectangle(mSrcFrame3, r, red); // Plot the target bounding box.


			if (target[i].LastTrackingFrame < frame - 5) // If the target is missing for 5 frames, remove it.
			{
				cvReleaseKalman(&(target[i].Kalman));
				for (int j = i; j < targetNum; j++)
				{
					target[j] = target[j + 1];
				}
				target[targetNum].Kalman = cvCreateKalman(4, 2, 0);
				const CvMat* prediction = cvKalmanPredict(target[targetNum].Kalman, 0);
				CvMat* process_noise = cvCreateMat(4, 1, CV_32FC1);
				CvRNG rng = cvRNG(-1);

				float a[4][4] = {//Transition matrix
					1,0,1,0,
					0,1,0,1,
					0,0,1,0,
					0,0,0,1
				};
				memcpy(target[targetNum].Kalman->transition_matrix->data.fl, a, sizeof(a));
				cvSetIdentity(target[targetNum].Kalman->measurement_matrix, cvRealScalar(1));
				cvSetIdentity(target[targetNum].Kalman->process_noise_cov, cvRealScalar(1e-5));
				cvSetIdentity(target[targetNum].Kalman->measurement_noise_cov, cvRealScalar(1e-1));
				cvSetIdentity(target[targetNum].Kalman->error_cov_post, cvRealScalar(1));
				cvReleaseMat(&process_noise);
				targetNum--;
				i--;
				continue;
			}

		}

		tempTargetNum = 0;
		namedWindow("track");
		moveWindow("track", 600, 50);
		imshow("track", mSrcFrame3);
	}

	cvReleaseVideoWriter(&pWriterTracking);
	
	return 0;
}

