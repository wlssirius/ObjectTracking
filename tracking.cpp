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