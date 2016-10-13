/**
 * OpenCV video streaming over TCP/IP
 * Server: Captures video from a webcam and send it to a client
 * by Isaac Maia
 */

//#include "opencv2/opencv.hpp"
#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <pigpio.h>
#include <thread>
#include <mutex>
#include "receiver.h"
#include "transmit.h"
#include "GPIOClass.h"

//using namespace cv;
using namespace std;


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


GPIOClass* frontLeft1; 
GPIOClass* frontLeft2;
GPIOClass* frontRight1;
GPIOClass* frontRight2;
GPIOClass* rearLeft1;
GPIOClass* rearLeft2;
GPIOClass* rearRight1; 
GPIOClass* rearRight2;

unsigned char output[DATAGRAM_SIZE];

void idle(){
	cout<< "IDLE" << endl;
	gpioWrite(19, 1);
	gpioWrite(26, 1);
	gpioWrite(4 , 1);
	gpioWrite(17, 1);
	gpioWrite(6 , 1);
	gpioWrite(13, 1);
	gpioWrite(2 , 1);
	gpioWrite(3 , 1);

}

void forward(){
	cout << "FORWARD" << endl;
	gpioWrite(19, 0);
	gpioWrite(26, 1);
	gpioWrite(4 , 0);
	gpioWrite(17, 1);
	gpioWrite(6 , 0);
	gpioWrite(13, 1);
	gpioWrite(2 , 0);
	gpioWrite(3 , 1);
	transmit(output);
}

void back(){
	cout << "BACK" << endl;
	gpioWrite(19, 1);
	gpioWrite(26, 0);
	gpioWrite(4 , 1);
	gpioWrite(17, 0);
	gpioWrite(6 , 1);
	gpioWrite(13, 0);
	gpioWrite(2 , 1);
	gpioWrite(3 , 0);
	transmit(output);
}

void left(){
	cout << "LEFT" << endl;
	gpioWrite(19, 1);
	gpioWrite(26, 0);
	gpioWrite(4 , 0);
	gpioWrite(17, 1);
	gpioWrite(6 , 1);
	gpioWrite(13, 0);
	gpioWrite(2 , 0);
	gpioWrite(3 , 1);
	transmit(output);
}

void right(){
	cout << "RIGHT" << endl;
	gpioWrite(19, 0);
	gpioWrite(26, 1);
	gpioWrite(4 , 1);
	gpioWrite(17, 0);
	gpioWrite(6 , 0);
	gpioWrite(13, 1);
	gpioWrite(2 , 1);
	gpioWrite(3 , 0);
	transmit(output);
}
void idlePWM(){
	cout<< "IDLE" << endl;
	gpioPWM(19, 255);
	gpioPWM(26, 255);
	gpioPWM(4 , 255);
	gpioPWM(17, 255);
	gpioPWM(6 , 255);
	gpioPWM(13, 255);
	gpioPWM(2 , 255);
	gpioPWM(3 , 255);

}

void forwardPWM(){
	cout << "FORWARD" << endl;
	/*gpioPWM(19, 0);
	gpioPWM(4 , 0);
	gpioPWM(6 , 0);
	gpioPWM(2 , 0);
	usleep(8000);
	gpioPWM(19, 20);
	gpioPWM(4 , 20);
	gpioPWM(6 , 20);
	gpioPWM(2 , 20);
	usleep(5000);
	gpioPWM(19, 100);
	gpioPWM(4 , 100);
	gpioPWM(6 , 100);
	gpioPWM(2 , 100);
	usleep(4000);
	gpioPWM(19, 180);
	gpioPWM(4 , 180);
	gpioPWM(6 , 180);
	gpioPWM(2 , 180);
	usleep(2000);
	gpioPWM(19, 220);
	gpioPWM(4 , 220);
	gpioPWM(6 , 220);
	gpioPWM(2 , 220);
	usleep(1000);*/
	gpioPWM(19, 200);
	gpioPWM(4 , 200);
	gpioPWM(6 , 200);
	gpioPWM(2 , 200);
	usleep(900000);

	/*
	usleep(20000);
	gpioPWM(19, 150);
	gpioPWM(4 , 150);
	gpioPWM(6 , 150);
	gpioPWM(2 , 150);
	usleep(20000);
	gpioPWM(19, 200);
	gpioPWM(4 , 200);
	gpioPWM(6 , 200);
	gpioPWM(2 , 200);*/
	transmit(output);
}

void backPWM(){
	cout << "BACK" << endl;
	/*gpioPWM(26, 20);
	gpioPWM(17, 20);
	gpioPWM(13, 20);
	gpioPWM(3 , 20);
	usleep(1000);*/
	gpioPWM(26, 110);
	gpioPWM(17, 110);
	gpioPWM(13, 110);
	gpioPWM(3 , 110);
	usleep(20000);
	gpioPWM(26, 150);
	gpioPWM(17, 150);
	gpioPWM(13, 150);
	gpioPWM(3 , 150);
	usleep(20000);
	gpioPWM(26, 200);
	gpioPWM(17, 200);
	gpioPWM(13, 200);
	gpioPWM(3 , 200);
	transmit(output);
}

void leftPWM(){
	cout << "LEFT" << endl;
	/*gpioPWM(26, 20);
	gpioPWM(4 , 20);
	gpioPWM(13, 20);
	gpioPWM(2 , 20);
	usleep(1000);*/
	gpioPWM(26, 130);
	gpioPWM(4 , 130);
	gpioPWM(13, 130);
	gpioPWM(2 , 130);
	usleep(30000);

	/*
	usleep(10000);
	gpioPWM(26, 150);
	gpioPWM(4 , 150);
	gpioPWM(13, 150);
	gpioPWM(2 , 150);
	usleep(20000);
	gpioPWM(26, 200);
	gpioPWM(4 , 200);
	gpioPWM(13, 200);
	gpioPWM(2 , 200);*/

	transmit(output);
}

void rightPWM(){
	cout << "RIGHT" << endl;
	/*gpioPWM(19, 20);
	gpioPWM(17, 20);
	gpioPWM(6 , 20);
	gpioPWM(3 , 20);
	usleep(1000);*/
	gpioPWM(19, 130);
	gpioPWM(17, 130);
	gpioPWM(6 , 130);
	gpioPWM(3 , 130);
	usleep(30000);

	/*
	usleep(10000);
	gpioPWM(19, 150);
	gpioPWM(17, 150);
	gpioPWM(6 , 150);
	gpioPWM(3 , 150);
	usleep(20000);
	gpioPWM(19, 200);
	gpioPWM(17, 200);
	gpioPWM(6 , 200);
	gpioPWM(3 , 200);*/

	transmit(output);
}




static char* snd_PORT = "10024"; 
static char* rcv_PORT="10023";
static char* IP_ADDR="192.168.0.101";


std::mutex mtxLock;
void idle_thread(unsigned int *idle_counter){
	while(1){
		cout << "SEARCHING..... " << *idle_counter << endl;
		mtxLock.lock();
		(*idle_counter)++;
		if(*idle_counter == 4){
			*idle_counter = 0;
			left();
			usleep(150000);
			idle();
		}
		// cout << *idle_counter << endl;
		mtxLock.unlock();
		usleep(1000000);
	}
}


int requestFlag = 0;
int main(int argc, char** argv){
    unsigned char* receivedPacket;
    int bytes, opcode;
    receiver_init(rcv_PORT);
	transmit_init(IP_ADDR, snd_PORT);
	if (gpioInitialise() < 0)
	{
	  fprintf(stderr, "pigpio initialisation failed\n");
	  return 1;
	}

	/* Set GPIO modes */
	gpioSetMode(19, PI_OUTPUT);
	gpioSetMode(26, PI_OUTPUT);
	gpioSetMode(4, PI_OUTPUT);
	gpioSetMode(17, PI_OUTPUT);
	gpioSetMode(6, PI_OUTPUT);
	gpioSetMode(13, PI_OUTPUT);
	gpioSetMode(2, PI_OUTPUT);
	gpioSetMode(3, PI_OUTPUT);

	unsigned int idle_counter = 0;
	thread t(idle_thread, &idle_counter);

    while(1){
        receivedPacket = receiver();
        //cout << receivedPacket[0]<<endl;      
        if((receivedPacket[0] == CONTROL)){
            opcode = CONTROL;   
        }
		else if(receivedPacket[0] == DETECT){
			opcode = DETECT;
		}
        else{opcode = NULL;}

        switch(opcode){
            case CONTROL:{
            		mtxLock.lock();
            		idle_counter = 0;
            		mtxLock.unlock();
                    if((receivedPacket[1] == FORWARD)){
       		        	forward();
                    }
                    else if((receivedPacket[1] == BACK)){
                        back();
                    }
                    else if((receivedPacket[1] == LEFT)){
                        left();
                    }
                    else if((receivedPacket[1] == RIGHT)){
                        right();
                    }
					else if(receivedPacket[1] == SIG_INT){
						pthread_t handler = t.native_handle();
						pthread_cancel(handler);
					    gpioTerminate();
					    
						close_transmit();
						close_receiver();
						cout << "DISCONNECTED..." << endl;
						usleep(3000000);
						receiver_init(rcv_PORT);
			 			transmit_init(IP_ADDR, snd_PORT);
						if (gpioInitialise() < 0)
						{
						  fprintf(stderr, "pigpio initialisation failed\n");
						  return 1;
						}

						/* Set GPIO modes */
						gpioSetMode(19, PI_OUTPUT);
						gpioSetMode(26, PI_OUTPUT);
						gpioSetMode(4, PI_OUTPUT);
						gpioSetMode(17, PI_OUTPUT);
						gpioSetMode(6, PI_OUTPUT);
						gpioSetMode(13, PI_OUTPUT);
						gpioSetMode(2, PI_OUTPUT);
						gpioSetMode(3, PI_OUTPUT);

					}
					else if(receivedPacket[1] == SHUTDOWN){
						system("sudo shutdown now");
					}
                    else {
                        idle();
                    }
					usleep(20000);
					idle();
                    
                    break;  
                }
            case DETECT:{
            		mtxLock.lock();
            		idle_counter = 0;
            		mtxLock.unlock();
                    if((receivedPacket[1] == FORWARD)){
       		        	forwardPWM();
                    }
                    else if((receivedPacket[1] == BACK)){
                        backPWM();
                    }
                    else if((receivedPacket[1] == LEFT)){
                        leftPWM();
                    }
                    else if((receivedPacket[1] == RIGHT)){
                        rightPWM();
                    }
                    else {
                        idlePWM();
                    }
					usleep(20000);
					idlePWM();
                    
                    break;  
                }


            default:{}
        }
    }
    gpioTerminate();
	return 0;

}

