// Developed by Wei Lisi, tlsirius@163.com
// July 26th,2017  

#include "tracking.h"

Tracking::Tracking()//Class for the tracking targets
{
	nFrame = 0;
}

Tracking::~Tracking()
{

}

int Tracking::DisplayResult(int nTime)
{
	printf("Current frame: %d\n", nFrame);

	imshow("Origin Image", m_mSrcFrame3);
	imshow("otsu threshold", m_mThreshold);
	waitKey(nTime);

	return REPLY_OK;
}

int Tracking::GetFrameNum()
{
	return nFrame;
}

int Tracking::CreateResultVideo()
{
	m_pWriterTracking = new VideoWriter("tracking.wmv", VideoWriter::fourcc('W', 'M', 'V', '1'), 25, Size(m_mSrcFrame3.size().width, m_mSrcFrame3.size().height), 1);

	return REPLY_OK;
}

int Tracking::WriteResultVideo()
{
	//IplImage pWrite = IplImage(m_mSrcFrame3);
	//cvWriteFrame(m_pWriterTracking, &pWrite);   //Write the display image to the output video
	return REPLY_OK;
}

int Tracking::ReleaseResultVideo()
{
	//cvReleaseVideoWriter(&m_pWriterTracking);
	return REPLY_OK;
}

int Tracking::LoadSequence(char* pcFileName)
{
	m_pVideoCapture.open(pcFileName);
	if (!m_pVideoCapture.isOpened())
	{
		cout << "Error! Failed to load file:" << pcFileName << endl;
		return REPLY_ERR;
	}
	return REPLY_OK;
}


int Tracking::LoadNextFrame()
{
	if (!m_pVideoCapture.read(m_mSrcFrame3))
	{
		return REPLY_ERR;
	}
	else
	{
		nFrame = nFrame + 1;
		return REPLY_OK; 
	}

	return REPLY_OK;
}

int Tracking::ProcessImage()
{
	cvtColor(m_mSrcFrame3, m_mSrcFrame, COLOR_RGB2GRAY); //Convert 3 channel RGB image to Gray image

	Mat mGaussian;
	GaussianBlur(m_mSrcFrame, mGaussian, Size(9, 9), 0);  //Gaussian blur
	for (int i = 0; i < 5; i++)
	{
		GaussianBlur(mGaussian, mGaussian, Size(9, 9), 0);
	}

	Mat mGroundSkyTest;
	threshold(mGaussian, mGroundSkyTest, 0, 255, THRESH_OTSU);
	//Otsu thresholding the Gaussian blured image
	//Get the raw image for horizon detecion

	Mat mDiff;
	absdiff(m_mSrcFrame, mGaussian, mDiff); //absolute value of the difference between original image and Gaussian blured image

	GaussianBlur(mDiff, m_mDiffGaussian, Size(9, 9), 0); //Gaussian blur
	for (int i = 0; i < 1; i++)
	{
		GaussianBlur(m_mDiffGaussian, m_mDiffGaussian, Size(9, 9), 0);
	}
	Mat mOtsuThreshold;
	threshold(m_mDiffGaussian, m_mThreshold, 30, 255, THRESH_BINARY);
	return REPLY_OK;
}

int Tracking::DetectTarget()
{
	Mat mSub, mSubOtsuThreshold;
	Mat pThreshold = m_mThreshold.clone();
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(m_mThreshold, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);    // Search for connected domains in the result of otsu thresholding

	tempTarget.clear();
	for (int i=0;i<contours.size();i++)
	{
		double tmparea = fabs(contourArea(contours[i]));
		if (tmparea > MIN_TARGET_SIZE)	//If the area of the connected domain is smaller than 500, it is removed. 
		{
			//----------------------Local thresholding-------------------------
			Rect r = boundingRect(contours[i]);
			if (tmparea > MIN_TARGET_SIZE)	//If the area);
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

			drawContours(pThreshold, contours, i, CV_RGB(0, 255, 255));

			Mat pThresholdOpt;
			pThreshold.copyTo(pThresholdOpt);
			Mat roi = pThresholdOpt(r);
			mSubOtsuThreshold.copyTo(roi);

			waitKey(10);


			//--------------------------------Extracting target shape from local thresholding result----------------------

			vector<vector<Point> > contours1;
			vector<Vec4i> hierarchy1;
			//IplImage pErode = IplImage(pThreshold);

			findContours(pThresholdOpt, contours1, hierarchy1, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
			// Search for connected domains in the result of local otsu thresholding
			
			for (int j=0; j!= contours1.size(); j++)
			{
				double tmparea = fabs(contourArea(contours1[j]));

				Rect r = boundingRect(contours1[j]);

				if (tmparea < MIN_TARGET_SIZE) //If the area of the connected domain is smaller than dynamicArea, it is removed.  
				{
					continue;
				}

				// Save the detected target in tempTarget. 				
				Target newTarget;
					
				newTarget.x = r.x;
				newTarget.y = r.y;
				newTarget.width = r.width;
				newTarget.height = r.height;
				newTarget.LastTrackingFrame = nFrame;
				tempTarget.push_back(newTarget);				

			}
		}

	}
	return REPLY_OK;
}


int Tracking::TrackTarget()
{
	if (target.size() == 0) //If No targets have been tracked, then track all the targets in current frame
	{
		for (int i = 0; i < tempTarget.size(); i++)
		{
			Target newTarget(tempTarget[i]);
			target.emplace_back(tempTarget[i]);
		}
	}
	else // If some targets have been tracked in the last frame, try to match (based on position and size) these targets with the newly detected targets.
	{
		for (int i = 0; i < tempTarget.size(); i++)
		{
			int trackFlag = 0;
			for (int j = 0; j < target.size(); j++)
			{
				if (target[j].sameTarget(tempTarget[i])) //estimate if the targets being tracked match with the newly detected targets
				{
					if (abs(target[j].center().x - tempTarget[i].center().x) < 10 && abs(target[j].center().y - tempTarget[i].center().y) < 10 && abs(target[j].width - tempTarget[i].width) < 10 && abs(target[j].height - tempTarget[i].height) < 10)
					{
						//if matched, update the state of the target.
						target[j].x = tempTarget[i].x;
						target[j].y = tempTarget[i].y;
						target[j].width = tempTarget[i].width;
						target[j].height = tempTarget[i].height;
						Mat measurement = Mat::zeros(2, 1, CV_32FC1);
						measurement.at<float>(0) = float(target[j].x + target[j].width / 2 - 1);
						measurement.at<float>(1) = float(target[j].y + target[j].height / 2 - 1);
						target[j].kalman->correct(measurement);
						target[j].kalman->predict();
						target[j].LastTrackingFrame = nFrame;
					}
					trackFlag = 1;
				}
			}
			if (trackFlag == 0)//If no match is found, save it as a new tracking target.
			{
				Target newTarget(tempTarget[i]);
				target.emplace_back(tempTarget[i]);
			}
		}
	}
	auto it = target.begin();
	while (it != target.end())
	{
		Rect r;
		r.width = it->width;
		r.height = it->height;
		r.x = it->x;
		r.y = it->y;

		Scalar red(0, 0, 200);
		rectangle(m_mSrcFrame3, r, red); // Plot the target bounding box.


		if (it->LastTrackingFrame < nFrame - 5) // If the target is missing for 5 frames, remove it.
		{
			delete it->kalman;
			it = target.erase(it);
		}
		else
		{
			it++;
		}

	}

	return REPLY_OK;
}


Point findCenter(Mat Image, vector<Point> contour)
{
	// Find the center point of the given connected domain'contour'.
	Rect r = boundingRect(contour);

	Point center(r.x+0.5*r.width, r.y+0.5*r.height);
	//Image.cols;
	//if (r.x + r.width>Image.cols)
	//{
	//	r.width = Image.cols - r.x;
	//}
	//if (r.y + r.height>Image.rows)
	//{
	//	r.height = Image.rows - r.y;
	//}
	//if (r.height <= 0)
	//{
	//	r.height = 0;
	//}
	//if (r.width <= 0)
	//{
	//	r.width = 0;
	//}
	//Mat subr = Image(r);
	//Moments m(subr, 0);
	//M00 = cvGetSpatialMoment(&m, 0, 0);
	//center.x = (int)(cvGetSpatialMoment(&m, 1, 0) / M00);
	//center.y = (int)(cvGetSpatialMoment(&m, 0, 1) / M00);
	return center;
}