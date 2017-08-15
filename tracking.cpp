// Developed by Wei Lisi, tlsirius@163.com
// July 26th,2017  

#include "tracking.h"

Tracking::Tracking()//Class for the tracking targets
{
	nFrame = 0;
	targetNum = 0;
	tempTargetNum = 0; 
}

Tracking::~Tracking()
{

}

int Tracking::DisplayHelp()
{
	int nReply=0;
	cout << endl;
	cout << "------------------Object Tracking Test Program------------------" << endl;
	cout << "Input 1 to train objects" << endl;
	cout << "Input 2 to track targets" << endl;
	cout << "Input 3 to quit" << endl;
	cout << "Please choose:" << endl;
	cin >> nReply;
	while (nReply < 1 || nReply > 3)
	{
		cout << "Input 1 to train objects" << endl;
		cout << "Input 2 to track targets" << endl;
		cout << "Input 3 to quit" << endl;
		cout << "Please choose:" << endl;
		cin >> nReply;
	}
	return nReply;
}


int Tracking::DisplayResult(int nTime)
{
	printf("Current frame: %d\n", nFrame);

	imshow("Origin Image", m_mSrcFrame3);
	imshow("otsu threshold", m_mThreshold);
	cvWaitKey(nTime);

	return REPLY_OK;
}

int Tracking::GetFrameNum()
{
	return nFrame;
}

int Tracking::CreateResultVideo()
{
	m_pWriterTracking = cvCreateVideoWriter("tracking.wmv", CV_FOURCC('W', 'M', 'V', '1'), 25, cvSize(m_mSrcFrame3.size().width, m_mSrcFrame3.size().height), 1);

	return REPLY_OK;
}

int Tracking::WriteResultVideo()
{
	IplImage pWrite = IplImage(m_mSrcFrame3);
	cvWriteFrame(m_pWriterTracking, &pWrite);   //Write the display image to the output video
	return REPLY_OK;
}

int Tracking::ReleaseResultVideo()
{
	cvReleaseVideoWriter(&m_pWriterTracking);
	return REPLY_OK;
}

int Tracking::LoadSequence(char* pcFileName)
{
	m_pSrcCapture = cvCaptureFromFile(FILE_NAME);
	if (m_pSrcCapture == NULL)
	{
		cout << "Error! " << FILE_NAME << " Not Found!" << endl;
		return REPLY_ERR;
	}

	return REPLY_OK;
}


int Tracking::LoadNextFrame()
{
	IplImage * pSrcFrame  = cvQueryFrame(m_pSrcCapture);
	if (pSrcFrame == NULL)
	{
		return REPLY_ERR;
	}
	else
	{
		m_mSrcFrame3 = Mat(pSrcFrame);
		nFrame = nFrame + 1;
		return REPLY_OK; 
	}

}

int Tracking::ProcessImage()
{
	cvtColor(m_mSrcFrame3, m_mSrcFrame, CV_RGB2GRAY); //Convert 3 channel RGB image to Gray image

	Mat mGaussian;
	GaussianBlur(m_mSrcFrame, mGaussian, cvSize(9, 9), 0);  //Gaussian blur
	for (int i = 0; i < 5; i++)
	{
		GaussianBlur(mGaussian, mGaussian, cvSize(9, 9), 0);
	}

	Mat mGroundSkyTest;
	threshold(mGaussian, mGroundSkyTest, 0, 255, THRESH_OTSU);
	//Otsu thresholding the Gaussian blured image
	//Get the raw image for horizon detecion

	Mat mDiff;
	absdiff(m_mSrcFrame, mGaussian, mDiff); //absolute value of the difference between original image and Gaussian blured image

	GaussianBlur(mDiff, m_mDiffGaussian, cvSize(9, 9), 0); //Gaussian blur
	for (int i = 0; i < 1; i++)
	{
		GaussianBlur(m_mDiffGaussian, m_mDiffGaussian, cvSize(9, 9), 0);
	}
	Mat mOtsuThreshold;
	threshold(m_mDiffGaussian, m_mThreshold, 30, 255, THRESH_BINARY);
	return REPLY_OK;
}

int Tracking::DetectTarget()
{
	tempTargetNum = 0;
	Mat mSub, mSubOtsuThreshold;
	IplImage pTemp = IplImage(m_mThreshold);
	IplImage *pThreshold = cvCreateImage(cvGetSize(&pTemp), 8, 1);
	IplImage *pThresholdOpt = cvCreateImage(cvGetSize(&pTemp), 8, 1);
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
		if (tmparea > MIN_TARGET_SIZE)	//If the area of the connected domain is smaller than 500, it is removed. 
		{


			//----------------------Local thresholding-------------------------
			CvRect r = ((CvContour*)contour2)->rect;
			mSub = m_mDiffGaussian(Range(r.y, r.y + r.height), Range(r.x, r.x + r.width));
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
			int mWidth = (m_mDiffGaussian.size()).width;
			int mHeight = (m_mDiffGaussian.size()).height;
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

				if (tmparea < MIN_TARGET_SIZE) //If the area of the connected domain is smaller than dynamicArea, it is removed.  
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
					tempTarget[tempTargetNum].LastTrackingFrame = nFrame;
					tempTargetNum++;
				}

			}
		}

	}
	return REPLY_OK;
}


int Tracking::TrackTarget()
{
	if (targetNum == 0) //If No targets have been tracked, then track all the targets in current frame
	{
		for (int i = 0; i < tempTargetNum; i++)
		{
			target[i].x = tempTarget[i].x;
			target[i].y = tempTarget[i].y;
			target[i].width = tempTarget[i].width;
			target[i].height = tempTarget[i].height;
			target[i].LastTrackingFrame = nFrame;

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
						target[j].LastTrackingFrame = nFrame;
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
				target[targetNum].LastTrackingFrame = nFrame;
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
		rectangle(m_mSrcFrame3, r, red); // Plot the target bounding box.


		if (target[i].LastTrackingFrame < nFrame - 5) // If the target is missing for 5 frames, remove it.
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

	return REPLY_OK;
}


int Tracking::TrainObject(CvSVM * svm)
{

}

CvPoint findCenter(const IplImage * Image, CvSeq* contour)
{
	// Find the center point of the given connected domain'contour'.
	CvMoments m;
	CvMat mat;
	double M00;
	CvRect rect = cvBoundingRect(contour);

	CvRect r = ((CvContour*)contour)->rect;
	CvPoint center;
	if (r.x + r.width>Image->width)
	{
		r.width = Image->width - r.x;
	}
	if (r.y + r.height>Image->height)
	{
		r.height = Image->height - r.y;
	}
	if (r.height <= 0)
	{
		r.height = 0;
	}
	if (r.width <= 0)
	{
		r.width = 0;
	}
	CvMat *subr = cvGetSubRect(Image, &mat, r);
	cvMoments(subr, &m, 0);
	M00 = cvGetSpatialMoment(&m, 0, 0);
	center.x = (int)(cvGetSpatialMoment(&m, 1, 0) / M00);
	center.y = (int)(cvGetSpatialMoment(&m, 0, 1) / M00);
	return center;
}