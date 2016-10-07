/**
 * OpenCV video streaming over TCP/IP
 * Client: Receives video from server and display it
 * by Isaac Maia
 */

#define DATAGRAM_SIZE 8


/*************************************
        OP CODE
**************************************/

#define CONTROL 0x01
#define DETECT  0x02
#define VIDEO   0x03

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


#include "opencv2/opencv.hpp"
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "getChar.cpp"
#include "transmit.h"
#include "receiver.h"
#include "kbhit.cpp"
#include <termios.h>

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <opencv2/imgproc.hpp>
//#include <legacy/legacy.hpp>
#include <ctime>
#include <iomanip>
#include <signal.h>

#include <thread>
#include <mutex>
#include <deque>

using namespace cv;
using namespace std;

int keyIn;
int sokt;

int localSocket;
int remoteSocket;
int serverPort;
int doneFlag = 1;
int counter = 0;
static char* snd_PORT="10023";
static char* rcv_PORT="10024";
//static char* IP_ADDR="127.0.0.1";
static char* IP_ADDR="192.168.0.100";


void printH(Mat H);

bool sigIntFlag = false;

void sigint(int a){
	cout << "SIG INT Captured!" << endl;
	cout << "Closing Socket Connections" << endl;
	unsigned char output[DATAGRAM_SIZE];
	output[0] = CONTROL;
	output[1] = SIG_INT;
	transmit(output);
	close_receiver();
	close_transmit();
	sigIntFlag = true;
}

void* getch_thread(void* data){
    unsigned char output[DATAGRAM_SIZE];
	unsigned char* receivedPacket;
    int bytes;
	bool wrongKey = false;
    while(1){
		tcflush(STDIN_FILENO, TCIFLUSH);	
		keyIn = getch();
        switch(keyIn){
            case 'a':{
                cout <<"left"<<endl;
                output[0] = CONTROL;
                output[1] = LEFT;
                transmit(output);
                break;
            }
            case 's':{
                cout <<"back"<<endl;
                output[0] = CONTROL;
                output[1] = BACK;
                transmit(output);
                break;
            }
            case 'd':{
                cout <<"right"<<endl;
                output[0] = CONTROL;
                output[1] = RIGHT;
                transmit(output);
                break;
            }
            case 'w':{
                cout <<"forward"<<endl;
                output[0] = CONTROL;
                output[1] = FORWARD;
                transmit(output);
                break;
            }
            case 'c':{
				pthread_exit(0);
                break;
            }
			case 'h':{
				cout <<"Shut Down Pi..."<<endl;
				output[0] = CONTROL;
				output[1] = SHUTDOWN;
				transmit(output);
				break;
			}
            default:{
				cout <<"\n\n\n\n" ;
				cout << "================================" << endl;
                cout <<"W,A,S,D for Control." << endl <<
					"C to cancel."<<endl <<
					"H to Shutdown Pi." << endl;
				cout << "================================" << endl;
				cout <<"\n\n\n\n" ;
                wrongKey = true;
                break;
            }
        
        }
		if(wrongKey){
			wrongKey = false;
		}
		else{
			receivedPacket = receiver();
			cout << "ACK1!" << endl;
		}
    }
    pthread_exit(NULL);
}

std::mutex mtxCam;

void video_thread(VideoCapture* cap, Mat* frame){
	while(1){
		//cap->grab();
		mtxCam.lock();
		*cap >> *frame;
		mtxCam.unlock();
		imshow("Video Thread", *frame);
		waitKey(1);
	}
}

mutex mtxControl;

void control_thread(int currentPerimeter){
    unsigned char output[DATAGRAM_SIZE];
    output[0] = DETECT;
    output[1] = FORWARD;
    //while(1){
        //mtxControl.lock();
        if(currentPerimeter<1000){
            for(int i = 0; i < 1; i++){
                transmit(output);
                //usleep(1000);
            }
        }
        //mtxControl.unlock();
    //}
}

int main(int argc, char** argv)
{

    pthread_t  getch_t, video_t;


    transmit_init(IP_ADDR, snd_PORT);  
    receiver_init(rcv_PORT); 
    pthread_create(&getch_t, NULL, getch_thread, (void*)&keyIn);

	signal(SIGINT, sigint);

    //----------------------------------------------------------
    //OpenCV Code
    //----------------------------------------------------------
    cv::VideoCapture vcap;
	vcap.set(CV_CAP_PROP_BUFFERSIZE, 3);
    cv::Mat frame;

//    const std::string videoStreamAddress = "http://192.168.0.100:8090/?action=stream"; 
    const std::string videoStreamAddress = "http://192.168.0.100:8080/stream/video.mjpeg"; 
    /* it may be an address of an mjpeg stream, 
    e.g. "http://user:pass@cam_address:8081/cgi/mjpg/mjpg.cgi?.mjpg" */

    //open the video stream and make sure it's opened
    if(!vcap.open(videoStreamAddress)) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1; 
    }  


    Mat img_object = imread("target.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    Mat img_scene;
    int minHessian = 400;
    //SiftFeatureDetector detector(minHessian);
    //Ptr<FeatureDetector> detector = ORB::create(minHessian);
	SiftFeatureDetector detector(minHessian);
    vector< KeyPoint > keypoints_object, keypoints_scene;

    //detector->detect(img_object, keypoints_object);
    detector.detect(img_object, keypoints_object);

    //VideoCapture cap(0); // open the default camera
    //if(!cap.isOpened())  // check if we succeeded
    //    return -1; 
    //step 2
    //SiftDescriptorExtractor extractor;
    //Ptr<DescriptorExtractor> extractor = ORB::create();
	SiftDescriptorExtractor extractor;
    Mat descriptors_object, descriptors_scene;
//    extractor->compute(img_object, keypoints_object, descriptors_object);
    extractor.compute(img_object, keypoints_object, descriptors_object);

    vcap>>frame;
    //vcap.grab();
    thread t(video_thread, &vcap, &frame);

    int perimeter = 0;
    int biasX = 0;
    vector<int> accuPerimeter;
    vector<int> accuBias;
    vector< Point2f > fixed_corners(4);
    bool detectFlag = false;
    bool newUpdate = false;
    int currentPerimeter;
    deque<int*> biasQueue;
    //bool control_update;
    //control_update = false;

    


    while (1){

		if(sigIntFlag){
			pthread_cancel(getch_t);
			return 0;
		}

		if(!frame.empty()){
			mtxCam.lock();
			frame.copyTo(img_scene);
			mtxCam.unlock();
		}

		//vcap.retrieve(frame);
		//frame.copyTo(img_scene);

        //if(!vcap.read(img_scene)) {
        //    std::cout << "No frame" << std::endl;
        //    cv::waitKey();
        //}
        //-- Step 1: Detect the keypoints using SURF Detector
        //threshold for hessian keypoint detector used in SURF
        

        clock_t start, end;

        //detect keypoints
        start = clock();
        detector.detect(img_scene, keypoints_scene);
        end = clock();
        //cout << "Detect keypoints use " << (double)(end - start) / CLOCKS_PER_SEC << " secs" << endl;
        //-- Step 2: Calculate descriptors(feature vectors)
        
        //(¹Ï, ¯S¼xÂI, ¯S¼x¦V¶q)
        start = clock();
        extractor.compute(img_scene, keypoints_scene, descriptors_scene);
        end = clock();
        //cout << "Compute descriptors use " << (double)(end - start) / CLOCKS_PER_SEC << " secs" << endl;


        //-- Step 3: Matching descriptor vectors using FLANN matcher
        FlannBasedMatcher matcher;
        vector< DMatch > matches;
        if(descriptors_object.type()!=CV_32F) {
            descriptors_object.convertTo(descriptors_object, CV_32F);
        }
        
        if(descriptors_scene.type()!=CV_32F) {
            descriptors_scene.convertTo(descriptors_scene, CV_32F);
        }

		if(descriptors_object.empty() || descriptors_scene.empty()){
			continue;
		}
        //start = clock();
        matcher.match(descriptors_object, descriptors_scene, matches);
        //end = clock();
        //cout << "Match use " << (double)(end - start) / CLOCKS_PER_SEC << " secs" << endl;

        double max_dist = 0; double min_dist = 100;

        //-- Quick calculation of max and min distances between keypoints
        //cout << "\n\n" << descriptors_object.rows << "\n\n";
        for (int i = 0; i < descriptors_object.rows; ++i){
            double dist = matches[i].distance;
            if (dist < min_dist)
                min_dist = dist;
            if (dist > max_dist)
                max_dist = dist;
        }

        //cout << "Max dist : " << max_dist << "\n";
        //cout << "Min dist : " << min_dist << "\n";

        //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
        vector< DMatch > good_matches;

        for (int i = 0; i < descriptors_object.rows; ++i){

            if (matches[i].distance < 3 * min_dist)
                good_matches.push_back(matches[i]);
        }

        Mat img_matches;
        start = clock();
        drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
            good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
            vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
        end = clock();
        //cout << "Draw matches use " << (double)(end - start) / CLOCKS_PER_SEC << " secs" << endl;
        //-- Localize the object
        start = clock();
        vector< Point2f > obj;
        vector< Point2f > scene;

        for (int i = 0; i < good_matches.size(); ++i){
            //-- Get the keypoints from the good matches
            obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
            scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
        }
        Mat H;
        if(obj.size() > 3 && scene.size() > 3){
            H = findHomography(obj, scene, CV_RANSAC);
        }
        
        //-- Get the corners from the image_1 ( the object to be "detected" )
        if(!H.empty()){
            //printH(H);
    
            vector< Point2f > obj_corners(4);
            obj_corners[0] = cvPoint(0, 0);
            obj_corners[1] = cvPoint(img_object.cols, 0);
            obj_corners[2] = cvPoint(img_object.cols, img_object.rows);
            obj_corners[3] = cvPoint(0, img_object.rows);
            vector< Point2f > scene_corners(4);
    
            perspectiveTransform(obj_corners, scene_corners, H);


            int edge[4];
            edge[0] = abs(scene_corners[0].y - scene_corners[1].y) + abs(scene_corners[0].x - scene_corners[1].x);
            edge[1] = abs(scene_corners[1].y - scene_corners[2].y) + abs(scene_corners[1].x - scene_corners[2].x);
            edge[2] = abs(scene_corners[2].y - scene_corners[3].y) + abs(scene_corners[2].x - scene_corners[3].x);
            edge[3] = abs(scene_corners[3].y - scene_corners[0].y) + abs(scene_corners[3].x - scene_corners[0].x);
            //mtxControl.lock();
            currentPerimeter = edge[0] + edge[1] + edge[2] + edge[3];
            //mtxControl.unlock();
            int *currentBias = new int[4];
            currentBias[0] = scene_corners[0].x;
            currentBias[1] = scene_corners[1].x;
            currentBias[2] = scene_corners[2].x;
            currentBias[3] = scene_corners[3].x;


            //for(int i = 0; i < 4; i++){
            //  if(biasX[i] == 0){
            //      biasX[i] = currentBias[i];
            //  }
            //}

            //bias pipeline
            if(biasQueue.size() != 3){
                biasQueue.push_back(currentBias);
            }
            else{
                int* toDelete = biasQueue.front();
                delete toDelete;
                biasQueue.pop_front();
                biasQueue.push_back(currentBias);
                int biasDelta[4];
                int accuBiasDelta = 0;
                //cout << "SIZEE: " << biasQueue.size() << endl;
                //computer difference between every 2 frames
                float base_weight = 0.7;
                for(int i = 0; i < biasQueue.size(); i++){
                    if(i == 0){
                        continue;
                    }
                    else{
                        for(int j = 0 ; j < 4; j++){
                            //cout << "==" << endl;
                            //cout << "QUEUE1: " << biasQueue.at(i)[j] << endl;
                            //cout << "QUEUE2: " << biasQueue.at(i - 1)[j] << endl;
                            accuBiasDelta += abs(biasQueue.at(i)[j] - biasQueue.at(i - 1)[j])
                                                *(base_weight + j*0.3);
                        }                       
                    }
                }

                //mtxControl.lock();
                if(currentPerimeter < 100 || accuBiasDelta > 400){
                    detectFlag = false;
                    fixed_corners.clear();
                    line(img_matches, scene_corners[0] + Point2f(img_object.cols, 0), scene_corners[1] + Point2f(img_object.cols, 0), Scalar(0, 0, 255), 4);
                    line(img_matches, scene_corners[1] + Point2f(img_object.cols, 0), scene_corners[2] + Point2f(img_object.cols, 0), Scalar(0, 0, 255), 4);
                    line(img_matches, scene_corners[2] + Point2f(img_object.cols, 0), scene_corners[3] + Point2f(img_object.cols, 0), Scalar(0, 0, 255), 4);
                    line(img_matches, scene_corners[3] + Point2f(img_object.cols, 0), scene_corners[0] + Point2f(img_object.cols, 0), Scalar(0, 0, 255), 4);
                }
                else{//update corners
                    detectFlag = true;
                    fixed_corners = scene_corners;
                }
                //mtxControl.unlock();
            }   
    
            if(fixed_corners.size() != 0){
                //-- Draw lines between the corners (the mapped object in the scene - image_2 )
                line(img_matches, fixed_corners[0] + Point2f(img_object.cols, 0), fixed_corners[1] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
                line(img_matches, fixed_corners[1] + Point2f(img_object.cols, 0), fixed_corners[2] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
                line(img_matches, fixed_corners[2] + Point2f(img_object.cols, 0), fixed_corners[3] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
                line(img_matches, fixed_corners[3] + Point2f(img_object.cols, 0), fixed_corners[0] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
            }
            
            end = clock();

            int bias = (fixed_corners[1].x - fixed_corners[0].x)/2 + fixed_corners[0].x;

            if(detectFlag){
                //mtxControl.lock();
                //control_update = true;
                //mtxControl.unlock();
                thread ctrl_t(control_thread, currentPerimeter);
                ctrl_t.detach();
                //cout << "BIAS: " << bias << endl;
                int biasFromCenter = bias - (img_matches.cols - img_object.cols)/2;
                //cout << "BIAS From Center: " << biasFromCenter << endl;
                unsigned char output[DATAGRAM_SIZE];
                output[0] = DETECT;
                if(biasFromCenter < 0){//object is at left
                    output[1] = LEFT;
                    cout << "LEFT!" << endl;
                    transmit(output);
                }
                else if(biasFromCenter > 0){//object is at right
                    output[1] = RIGHT;
                    cout << "RIGHT!" << endl;
                    transmit(output);
                }

                //if(currentPerimeter < 1000){
                //    output[1] = FORWARD;
                //    cout << "FORWARD!" << endl;
                //    for(int i = 0; i < 2; i++){
                //        transmit(output);
                //    }
                //}
            }
            else{
                //mtxControl.lock();
                //control_update = false;
                //mtxControl.unlock();
            }
            //cout << "Localize the object use " << (double)(end - start) / CLOCKS_PER_SEC << " secs" << endl;
            //-- Show detected matches
            imshow("Good Matches & Object detection", img_matches);
    
            waitKey(1);
        }
        
    } 

    return 0;
}

void printH(Mat H)
{
    for (int i = 0; i < 3; ++i){
        for (int j = 0; j < 3; ++j){
            cout << ::left << ::setw(15) << ::setfill(' ') << H.at< double >(i, j);
            //cout << H.at< double >(i, j);
            //cout << "\t\t";
        }
        cout << "\n";
    }
}
	
