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
#define ULTRA   0x03
#define ACK     0x04
#define	DUMMY	0x05

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
#define HONK 	0x07

/**************************
        VIDEO
***************************/
#define START   0x00
#define VALID   0x01
#define REQUEST 0x02
#define STOP    0x03
#define SENT    0x04

#define TRIG 23
#define ECHO 24

/* Ultrasonic */
int last_range = 0;	     // last sonar reading

/* Interrupt from ultrasonic to movement of car */
bool breakFlag = false;
std::mutex breakLock;

/* Interrupt from Control to Autonomous Driving */
bool breakDetect = false;
std::mutex breakDetectLock;
////////////////////

int opcode = DUMMY;
std::mutex opcodeLock;

unsigned char* receivedPacket;
/*
GPIOClass* frontLeft1; 
GPIOClass* frontLeft2;
GPIOClass* frontRight1;
GPIOClass* frontRight2;
GPIOClass* rearLeft1;
GPIOClass* rearLeft2;
GPIOClass* rearRight1; 
GPIOClass* rearRight2;*/

/* Output Packet to be sent */
unsigned char output[DATAGRAM_SIZE];

/* Directional movement of vehicle based on user input */
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
void *honk_thread(void* arg){
	cout << "HONK" << endl;

	int honkDuration = 10;
	for(int i = 0; i < honkDuration; i++){
		gpioWrite(21, 1);
		usleep(7000);
		gpioWrite(21, 0);
		usleep(7000);
	}
	transmit(output);
}

void forward(){
	if(!breakFlag){
		cout << "FORWARD" << endl;
		gpioWrite(19, 0);
		gpioWrite(26, 1);
		gpioWrite(4 , 0);
		gpioWrite(17, 1);
		gpioWrite(6 , 0);
		gpioWrite(13, 1);
		gpioWrite(2 , 0);
		gpioWrite(3 , 1);
	}
	else
		cout << "Too Near!" << endl;
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

/* Directional Movement of Vehicle based on tracked object */
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

	if(!(breakFlag || breakDetect) ){
		cout << "FORWARD" << endl;
		gpioPWM(19, 200);
		gpioPWM(4 , 200);
		gpioPWM(6 , 200);
		gpioPWM(2 , 200);
		for(int i = 0; i < 9; i++){
			if(breakFlag || breakDetect)
				break;
			usleep(30000);
		}
	}
	else
		cout << "Too Near!" << endl;

	transmit(output);
}

void backPWM(){
	cout << "BACK" << endl;

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
	gpioPWM(26, 130);
	gpioPWM(4 , 130);
	gpioPWM(13, 130);
	gpioPWM(2 , 130);
	usleep(30000);

	transmit(output);
}

void rightPWM(){
	cout << "RIGHT" << endl;

	gpioPWM(19, 130);
	gpioPWM(17, 130);
	gpioPWM(6 , 130);
	gpioPWM(3 , 130);
	usleep(30000);


	transmit(output);
}


static char* snd_PORT = "10024"; 
static char* rcv_PORT="10023";
static char* IP_ADDR="192.168.0.101";

/* Search for Object by Rotating */
std::mutex mtxLock;
void* idle_thread(void* arg){
	unsigned int* idle_counter = (unsigned int*)arg;
	while(1){
		cout << "SEARCHING..... " << *idle_counter << endl;
		mtxLock.lock();
		(*idle_counter)++; // increase idle counter when object not found
		if(*idle_counter == 4){
			*idle_counter = 0;
			left();
			usleep(150000);
			idle();
		}
		mtxLock.unlock();
		usleep(1000000);
	}
}

//////////////////////////////////////////////

volatile uint32_t before;  // used for sonar distance


void delay(int ms) {  // delay in miliseconds
    gpioDelay(1000*ms); 
}

void ping(void) {     // send out an ultrasonic 'ping'

   before = 0xffffffff; // set for guard variable

   gpioSetMode(TRIG, PI_OUTPUT);

   // trigger a sonar pulse
   
   gpioWrite(TRIG, PI_OFF);
   gpioDelay(5);
   gpioWrite(TRIG, PI_ON);
   gpioDelay(10); 	
   gpioWrite(TRIG, PI_OFF);
   gpioDelay(5);

   gpioSetMode(ECHO, PI_INPUT);

   before = gpioTick(); // get tick right after sending pulse
   
}

// range - callback function for measuring ping response

void range(int gpio, int level, uint32_t tick) {

   static uint32_t startTick, endTick;
   pthread_t honk_t;
   uint32_t diffTick;

   if (tick>before) { // make sure we don't measure trigger pulse

      if (level == PI_ON) { // start counting on rising edge
         startTick = tick;
      }  
      else if (level == PI_OFF) { // stop counting on falling edge

         endTick = tick;
         diffTick = (endTick - startTick)/58;

         last_range = diffTick;

         breakLock.lock();
         if(last_range>15)
         	breakFlag = false;
         else{
         	breakFlag = true;
			pthread_create(&honk_t, NULL, honk_thread, NULL);
         }
         breakLock.unlock();

         if (diffTick < 600)
            printf("%u\n", diffTick);
         else {
            printf("OUT OF RANGE"); // for seeedstudio sensor
            last_range = 0;
         }
      }

   }

}

void sleep(int t) {
  gpioSleep(PI_TIME_RELATIVE, t, 0);
}

/* Thread for ultrasonic sensor */
void* ultra_thread(void* arg){
 	unsigned char output_ultra[DATAGRAM_SIZE];

 	output_ultra[0] = ULTRA; // Sets Packet Header

	gpioSetMode(ECHO, PI_INPUT); // Initialize pin as input pin

	// register callback on change of sonar pin
	gpioSetAlertFunc(ECHO, range); 

	sleep(2);    
	while (1) {

		ping();	// prime the last_range variable
		// sleep(1);
		usleep(500000);
		// Pack OBSTACLE RANGE into a single packet
		output_ultra[1] = last_range & 0xFF;
		output_ultra[2] = (last_range >> 8) & 0xFF;
		output_ultra[3] = (last_range >> 16) & 0xFF;
		output_ultra[4] = (last_range >> 24) & 0xFF;
		transmit(output_ultra); // update controller with the range of object
	}
 }

/* Thread to receive command from Controller */
 void* forward_thread(void* arg){
	
    if((receivedPacket[1] == FORWARD)){
		// pthread_create(&forward_t, NULL, forward_thread, &idle_counter);
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
	// usleep(20000);
	idlePWM();
 }
//////////////////////////////////////////////

int main(int argc, char** argv){
    int bytes;
    output[0] = ACK; // Initialize output packet header as ACK
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
	gpioSetMode(21, PI_OUTPUT);

	unsigned int idle_counter = 0;
	pthread_t idle_t, ultra_t, forward_t, honk_t;
	pthread_create(&idle_t, NULL, idle_thread, &idle_counter);
	pthread_create(&ultra_t, NULL, ultra_thread, &idle_counter);
    while(1){
    	// if (opcode == DUMMY)
    		// continue;

	    receivedPacket = receiver();
	    if((receivedPacket[0] == CONTROL)){
	    	breakDetect = true;
	        opcode = CONTROL;   
	    }
		else if(receivedPacket[0] == DETECT)
			opcode = DETECT;
	    else
	    	opcode = NULL;
        
		pthread_join(forward_t, NULL);

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
                    else if((receivedPacket[1] == HONK)){
						pthread_create(&honk_t, NULL, honk_thread, &idle_counter);
                    }
					else if(receivedPacket[1] == SIG_INT){
						if(pthread_cancel(idle_t))
							cout << "idle_t cancel fail..." << endl;
						else
							cout << "idle_t cancel SUCCESS!" << endl;

						if(pthread_cancel(ultra_t))
							cout << "ultra_t cancel fail..." << endl;
						else
							cout << "ultra_t cancel SUCCESS!" << endl;

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
						gpioSetMode(21, PI_OUTPUT);
						pthread_create(&idle_t, NULL, idle_thread, &idle_counter);
						pthread_create(&ultra_t, NULL, ultra_thread, &idle_counter);
					}
					else if(receivedPacket[1] == SHUTDOWN){
						system("sudo shutdown now");
					}
                    else {
                        idle();
                    }
					usleep(20000);
					idle();
                    
                    breakDetect = false;
                    break;  
                }
            case DETECT:{
            		mtxLock.lock();
            		idle_counter = 0;
            		mtxLock.unlock();
    				pthread_create(&forward_t, NULL, forward_thread, &idle_counter);

                    break;  
                }


            default:{}
        }
    }
    gpioTerminate();
	return 0;

}

