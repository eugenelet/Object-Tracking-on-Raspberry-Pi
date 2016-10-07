/**
 * OpenCV video streaming over TCP/IP
 * Client: Receives video from server and display it
 * by Isaac Maia
 */

#define DATAGRAM_SIZE 1024


/*************************************
        OP CODE
**************************************/

#define CONTROL 0x01
#define PICTURE 0x02
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
#include "transmit.cpp"

using namespace cv;
using namespace std;

int keyIn;
int sokt;

int localSocket;
int remoteSocket;
int serverPort;
int doneFlag = 1;
int counter = 0;
static char* snd_PORT="2023";
int rcv_PORT=2022;
//static char* IP_ADDR="127.0.0.1";
static char* IP_ADDR="192.168.2.100";


void* getch_thread(void* data){
    unsigned char output[DATAGRAM_SIZE];
    int bytes;
    while(1){
        keyIn = getch();
        //getch_flag = true;
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
            case 'v':{
               //  //   transmit(output);
               // }
                break;
            }
            default:{
                cout <<"idle"<<endl;
                output[0] =  CONTROL;
                output[1] = IDLE;
                transmit(output);
                break;
            }
        
        }
    }
    pthread_exit(NULL);
}



int main(int argc, char** argv)
{

    pthread_t  getch_t, video_t;


	transmit_init(IP_ADDR, snd_PORT);    
    pthread_create(&getch_t, NULL, getch_thread, (void*)&keyIn);



    for(;;) {
	}
    return 0;
}
	
