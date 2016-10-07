#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"

#include "receiver.h"
#include "transmit.h"

#include <cmath>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <set>
#include <signal.h>
#include <termios.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h> /*strtol*/
#include <dirent.h>

#include <thread>
#include <mutex>
#include <deque>


#define DATAGRAM_SIZE 8


/*************************************
        OP CODE
**************************************/

#define CONTROL 0x01
#define DETECT  0x02
#define ULTRA   0x03
#define ACK     0x04

/*************************************
        FUNCTION
**************************************/


/**************************
        CONTROL
***************************/
#define IDLE    0x00
#define FORWARD 0x01
#define BACK    0x02
#define LEFT    0x03
#define RIGHT   0x04
#define SIG_INT 0x05
#define SHUTDOWN 0x06

/**************************
        VIDEO
***************************/
#define START   0x00
#define VALID   0x01
#define REQUEST 0x02
#define STOP    0x03
#define SENT    0x04


/**************************
        SIFT TOOLS
***************************/
#define PI 3.14159265358979323846264338327
#define SIFT_ORI_HIST_BINS 36// default number of bins in histogram for orientation assignment
#define SIFT_ORI_SIG 1.5f// determines gaussian sigma for orientation assignment
#define SIFT_ORI_RADIUS 3 * SIFT_ORI_SIG// determines the radius of the region used in orientation assignment
#define SCALE 1.6
#define DOG_DETECT_KPT_SIZE 3

using namespace cv;
using namespace std;


class Key_Point;
class mySIFT;
class OMG;
typedef vector< vector<double> > double2D;
void match(mySIFT& left, mySIFT& right, string targetFile, Mat img_scene, clock_t s);
Mat concat2Img(Mat& i1, Mat& i2);
Point2f MatMulti(Mat matrix, Point2f point);
void computeSift(mySIFT &sift_obj, Mat img_scene, Mat img_color, bool time_on);
void computeSift_left(mySIFT &sift_obj, Mat img_scene, Mat img_color, bool time_on);
void calculateR(Mat& R, Mat& Dxx, Mat& Dxy, Mat& Dyy);
void computeDxxDxyDyy(Mat& src, Mat& Dxx, Mat& Dxy, Mat& Dyy);
void match_multi(mySIFT* left, mySIFT& right, vector<char*> targetFile, Mat img_scene, int target_num, int target_pick);
Mat concatMultiImg(Mat* target, Mat& scene, int& target_num, int& maxCol, int& target_pick);
char getch();
void trackObject(vector<Point2f> &computed_corners, Mat &result, mySIFT &left, mySIFT &right, int &maxCol);

class Key_Point{
public:
	Key_Point(double s, int r, int c, int t, int l) : scale(s), row(r), col(c), type(t), layer(l){}
	int cornerValue;
	int row;
	int col;
	int orientation;
	int type;//0:min, 1:max
	double scale;
	int layer;//¦bDoGªº­þ¤@­ÓLayer
	vector< double > descriptor;
	float R;
};

class OMG{
public:
	int row;
	int col;
	float Rvalue;
	Key_Point myKeyPoint;

	OMG(int r, int c, float R, Key_Point& inKey) : row(r), col(c), Rvalue(R), myKeyPoint(inKey){}
	bool operator<(const OMG& p) const { return this->Rvalue < p.Rvalue; }
};


class mySIFT{
public:
	mySIFT(){}
	mySIFT(double sig, double kk, int nOct = 2, int nLpO = 5) : nOctave(nOct), nLayersPerOctave(nLpO), sigma(sig), k(kk){}
	void createDoG();
	void detectKeypoints();
	void LoadImage(Mat imgOri);
	void filterKeyPoints();
	void filterKeyPoints2();
	void drawKeyPoints(string imageName);
	void computeDescriptor();
	vector< Key_Point > keyPoints;
	vector< Key_Point > filteredKeyPoints;
	vector< Mat > blurredImgs;

	double2D getGaussianKernel(double sigma);
	Mat GaussianBlur(const Mat& src, double sigma);
	void computeOrientationHist(vector< Key_Point >& keyPoints);
	vector< double > DescriptorHelper(int row, int column, int layer);
	void filterKeyPointsHelper1(vector< int >& brighter, vector< int >& darker, Mat& thisMat, Key_Point& thisKpt, int value, int threshold);
	void filterKeyPointsHelper2(vector< int >& brighter, vector< int >& darker, int& nbrighter, int& ndarker);
	void filterKeyPoints_Hessian(Mat img_scene, Mat img_color);
	void filterKeyPoints_Hessian_left(Mat img_scene, Mat img_color);

	vector< Mat > DoGs;
	int nLayersPerOctave;
	int nOctave;
	double sigma;
	double k;
};
