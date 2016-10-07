#include "../include/sift.h"

void mySIFT::filterKeyPoints()
{
	for (int i = 0; i < keyPoints.size(); ++i){
		Key_Point& thisKpt = keyPoints[i];
		Mat& thisMat = blurredImgs[thisKpt.layer];
		//¦³n1­Ó³sÄòªºpixel³£¤ñthisKpt²`¡A¦³n2­Ó³sÄòªºÂI³£¤ñthisKpt²L
		
		//¦bDoG¤W±o¨ìªºkeyPoint¤£·|¦bÃä¬É
		//if (thisKpt.row - 3 < 0 || thisKpt.row + 3 >= thisMat.rows || thisKpt.col - 3 < 0 || thisKpt.col + 3 >= thisMat.cols)//ÀË¬d½d³ò¡A¤£¯à°÷µe¾ã°éªº´Nª½±µÙT
		//	continue;
		vector< int > brighter(8, 0);
		vector< int > darker(8, 0);
		int value = thisMat.at<uchar>(thisKpt.row, thisKpt.col);
		int threshold = 7;
		filterKeyPointsHelper1(brighter, darker, thisMat, thisKpt, value, threshold);
		int nBrighter, nDarker;
		filterKeyPointsHelper2(brighter, darker, nBrighter, nDarker);
		thisKpt.cornerValue = max(nBrighter, nDarker);
		if (thisKpt.cornerValue > 8 || thisKpt.cornerValue < 0)
			cout << "??\n";
	}

	//°µsortingºO¡A¤jªº±Æ«e­±
	//for (int i = 1; i < keyPoints.size(); ++i)
	//	for (int j = i - 1; j >= 0; --j)
	//		if (keyPoints[j + 1].cornerValue > keyPoints[j].cornerValue)
	//			swap(keyPoints[j + 1], keyPoints[j]);
	//vector< Key_Point > temp(keyPoints.begin(), (keyPoints.size() < 500) ? keyPoints.end() : keyPoints.begin() + 500);
	//cout << keyPoints.size() << " ";
	vector< Key_Point > temp;
	int layer1 = 0;
	int layer2 = 0;
	for (int i = 0; i < keyPoints.size(); ++i){
		if (keyPoints[i].cornerValue >= 4){
			temp.push_back(keyPoints[i]);
			if (keyPoints[i].layer == 1)
				++layer1;
			else if (keyPoints[i].layer == 2)
				++layer2;
			else{
				if (this->blurredImgs.size() == 5)
					cout << "FUCK\n";
			}
		}
	}
	// cout << temp.size() << " " << layer1 << " " << layer2 << "\n";
	keyPoints = temp;
}

void mySIFT::filterKeyPointsHelper1(vector< int >& brighter, vector< int >& darker, Mat& thisMat, Key_Point& thisKpt, int value, int threshold)
{
	//§Ëbrighter
	int brighterthshod = value + threshold;
	if (thisMat.at<uchar>(thisKpt.row - 1, thisKpt.col - 1) > brighterthshod)
		brighter[0] = 1;
	if (thisMat.at<uchar>(thisKpt.row - 1, thisKpt.col) > brighterthshod)
		brighter[1] = 1;
	if (thisMat.at<uchar>(thisKpt.row - 1, thisKpt.col + 1) > brighterthshod)
		brighter[2] = 1;
	if (thisMat.at<uchar>(thisKpt.row, thisKpt.col - 1) > brighterthshod)
		brighter[3] = 1;
	if (thisMat.at<uchar>(thisKpt.row, thisKpt.col + 1) > brighterthshod)
		brighter[4] = 1;
	if (thisMat.at<uchar>(thisKpt.row + 1, thisKpt.col - 1) > brighterthshod)
		brighter[5] = 1;
	if (thisMat.at<uchar>(thisKpt.row + 1, thisKpt.col) > brighterthshod)
		brighter[6] = 1;
	if (thisMat.at<uchar>(thisKpt.row + 1, thisKpt.col + 1) > brighterthshod)
		brighter[7] = 1;

	//§Ëdarker
	int darkerthshod = value - threshold;
	if (thisMat.at<uchar>(thisKpt.row - 1, thisKpt.col - 1) < darkerthshod)
		darker[0] = 1;
	if (thisMat.at<uchar>(thisKpt.row - 1, thisKpt.col) < darkerthshod)
		darker[1] = 1;
	if (thisMat.at<uchar>(thisKpt.row - 1, thisKpt.col + 1) < darkerthshod)
		darker[2] = 1;
	if (thisMat.at<uchar>(thisKpt.row, thisKpt.col - 1) < darkerthshod)
		darker[3] = 1;
	if (thisMat.at<uchar>(thisKpt.row, thisKpt.col + 1) < darkerthshod)
		darker[4] = 1;
	if (thisMat.at<uchar>(thisKpt.row + 1, thisKpt.col - 1) < darkerthshod)
		darker[5] = 1;
	if (thisMat.at<uchar>(thisKpt.row + 1, thisKpt.col) < darkerthshod)
		darker[6] = 1;
	if (thisMat.at<uchar>(thisKpt.row + 1, thisKpt.col + 1) < darkerthshod)
		darker[7] = 1;
}

void mySIFT::filterKeyPointsHelper2(vector< int >& brighter, vector< int >& darker, int& nBrighter, int& nDarker)
{
	if (brighter.size() != 8 || darker.size() != 8)
		cout << "?????\n";

	for (int i = 0; i < 4; ++i)
		brighter.push_back(brighter[i]);

	int accu1 = 0;
	int maxAccu1 = 0;
	for (int i = 0; i < 8; ++i){
		if (brighter[i]){
			++accu1;
			if (accu1 > maxAccu1)
				++maxAccu1;
		}
		else
			accu1 = 0;//Âk0
	}
	nBrighter = maxAccu1;

	accu1 = 0;
	maxAccu1 = 0;
	for (int i = 4; i < 12; ++i){
		if (brighter[i]){
			++accu1;
			if (accu1 > maxAccu1)
				++maxAccu1;
		}
		else
			accu1 = 0;
	}
	if (maxAccu1 > nBrighter)
		nBrighter = maxAccu1;

	for (int i = 0; i < 4; ++i)
		darker.push_back(darker[i]);

	accu1 = 0;
	maxAccu1 = 0;
	for (int i = 0; i < 8; ++i){
		if (darker[i]){
			++accu1;
			if (accu1 > maxAccu1)
				++maxAccu1;
		}
		else
			accu1 = 0;//Âk0
	}
	nDarker = maxAccu1;

	accu1 = 0;
	maxAccu1 = 0;
	for (int i = 4; i < 12; ++i){
		if (darker[i]){
			++accu1;
			if (accu1 > maxAccu1)
				++maxAccu1;
		}
		else
			accu1 = 0;
	}
	if (maxAccu1 > nDarker)
		nDarker = maxAccu1;
}