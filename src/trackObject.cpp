#include "../include/sift.h"

#define HIGH_THROUGHPUT 0
#define HIGH_ACCURACY   1

/* Mode for Auto Adjustment*/
int MODE = HIGH_ACCURACY;

/* System Filter Threshold */
extern int FILTER_THRESHOLD;

/* Feedback from Ultrasonic Sensor*/
extern int distance_obj;

/* System Mode for Threhsold (AUTO/MANUAL) */
extern int currentMode;

/* Validity of Target */
int perimeter = 0;
int biasX = 0;
vector<int> accuPerimeter;
vector<int> accuBias;
vector< Point2f > fixed_corners(4);
bool detectFlag = false;
bool newUpdate = false;
int currentPerimeter;
deque<int*> biasQueue;

void updateFilterThreshold(int mode, mySIFT& left, mySIFT& right);

void trackObject(vector<Point2f> &computed_corners, Mat &result, mySIFT &left, mySIFT &right, int &maxCol){

	int edge[4];
    edge[0] = abs(computed_corners[0].y - computed_corners[1].y) + abs(computed_corners[0].x - computed_corners[1].x);
    edge[1] = abs(computed_corners[1].y - computed_corners[2].y) + abs(computed_corners[1].x - computed_corners[2].x);
    edge[2] = abs(computed_corners[2].y - computed_corners[3].y) + abs(computed_corners[2].x - computed_corners[3].x);
    edge[3] = abs(computed_corners[3].y - computed_corners[0].y) + abs(computed_corners[3].x - computed_corners[0].x);
    currentPerimeter = edge[0] + edge[1] + edge[2] + edge[3];
    int *currentBias = new int[4];
    currentBias[0] = computed_corners[0].x;
    currentBias[1] = computed_corners[1].x;
    currentBias[2] = computed_corners[2].x;
    currentBias[3] = computed_corners[3].x;
    
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
        //computer difference between every 2 frames
        for(int i = 0; i < biasQueue.size(); i++){
            if(i == 0){
                continue;
            }
            else{
                for(int j = 0 ; j < 4; j++){
                    accuBiasDelta += abs(biasQueue.at(i)[j] - biasQueue.at(i - 1)[j]);
                }
            }
        }
        if(currentPerimeter < 100 || accuBiasDelta > 400){
            detectFlag = false;
            fixed_corners.clear();
            line(result, computed_corners[0], computed_corners[1], Scalar(0, 0, 255), 4);
            line(result, computed_corners[1], computed_corners[2], Scalar(0, 0, 255), 4);
            line(result, computed_corners[2], computed_corners[3], Scalar(0, 0, 255), 4);
            line(result, computed_corners[3], computed_corners[0], Scalar(0, 0, 255), 4);
        }
        else{//update corners
            detectFlag = true;
            fixed_corners = computed_corners;
        }
    }
    if(fixed_corners.size() != 0){
        //-- Draw lines between the corners (the mapped object in the scene - image_2 )
        line(result, fixed_corners[0], fixed_corners[1], Scalar(0, 255, 0), 4);
        line(result, fixed_corners[3], fixed_corners[0], Scalar(0, 255, 0), 4);
        line(result, fixed_corners[1], fixed_corners[2], Scalar(0, 255, 0), 4);
        line(result, fixed_corners[2], fixed_corners[3], Scalar(0, 255, 0), 4);
    }
    int bias = (fixed_corners[1].x - fixed_corners[0].x)/2 + fixed_corners[0].x;
    if(detectFlag){
        updateFilterThreshold(HIGH_THROUGHPUT, left, right);
        int biasFromCenter = bias - (maxCol + right.blurredImgs[0].cols/2);
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
        if(currentPerimeter < 1000){
            if(distance_obj>15){
                output[1] = FORWARD;
                cout << "FORWARD!" << endl;
                transmit(output);
            }
            else
                cout << "Object too near!" << endl;
        }
    }
    else
        updateFilterThreshold(HIGH_ACCURACY, left, right);
}

void updateFilterThreshold(int mode, mySIFT& left, mySIFT& right){
    vector< Key_Point >& a = left.keyPoints;
    vector< Key_Point >& b = right.keyPoints;
    if(currentMode == AUTO)
        MODE = mode;
    else{
        cout << "MANUAL| ";
        if(currentMode == HIGH_ACCURACY_MANUAL)
            MODE = HIGH_ACCURACY;
        else if(currentMode == HIGH_THROUGHPUT_MANUAL)
            MODE = HIGH_THROUGHPUT;
    }   

    if ((MODE && b.size() < 1500) || (!MODE && b.size() < 100))
        --FILTER_THRESHOLD;
    else if ((MODE && b.size() > 2000) || (!MODE && b.size() > 200))
        ++FILTER_THRESHOLD;

    switch(MODE){
        case HIGH_ACCURACY:
            cout << "MODE: HIGH_ACCURACY  " ;
            break;
        case HIGH_THROUGHPUT:
            cout << "MODE: HIGH_THROUGHPUT:  " ;
            break;
        default:
            cout << "ERROR..." << endl;
            break;
    }
    cout << "FILTER_THRESHOLD: " << FILTER_THRESHOLD << endl;
}