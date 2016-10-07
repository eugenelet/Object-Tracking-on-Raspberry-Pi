#include "../include/sift.h"

void mySIFT::drawKeyPoints(string imageName)
{
	Mat imgOriRGB = imread(imageName);
	for (int i = 0; i < keyPoints.size(); ++i){
		Key_Point& kpt = keyPoints[i];
		circle(imgOriRGB, Point(kpt.col, kpt.row), 5, Scalar(0, 0, 255));
		//line(imgOriRGB, Point(kpt.col, kpt.row), Point(kpt.col + 10 * kpt.scale * cos(kpt.orientation * PI / 180), kpt.row + 10 * kpt.scale * sin(kpt.orientation * PI / 180)), Scalar(203, 192, 255));
	}
	//line(Mat, Point, Point, Scalar)
	//imshow("Output", blurredImgs[0]);
	//imshow("Ori", imgOriRGB);
	//waitKey(10);
}