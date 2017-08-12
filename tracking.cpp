// Developed by Wei Lisi, tlsirius@163.com
// July 26th,2017  

#include "tracking.h"

Tracking::Tracking()//Class for the tracking targets
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

int Tracking::LoadSequence()
{

	return 0;
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