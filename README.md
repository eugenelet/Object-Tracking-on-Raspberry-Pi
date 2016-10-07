Object Detect with SIFT on Autonomous Vehicle using Raspberry Pi
================================================================

This is a program that runs on Raspberry Pi. The main goal of this program is to detect object based on SIFT. All complex computations which includes control of vehicle and SIFT are done remotely, which means that Raspberry Pi only acts as a system that receives commands from a remote computation unit to control our vehicle. 

SIFT is an algorithm that's can point out the location of an object in an image by using a bounding box wrapping the object. The matching is based on features extracted from SIFT. This version of SIFT is implemented by ourselves so it's relatively slower than the libraries provided by OpenCV, but based on our experiment (we are using Intel(R) Core(TM) i5-3320M CPU), it doesn't seem to be a problem in the tracking of an object by our vehicle.

The connection protocol between raspberry pi and our remote machine will be via WiFi.

Using this code, you'll be able to:
    - Control your vehicle using your keyboard (W,A,S,D)
    - Real-time detection of object(image) fed into this program

Requirements
------------
Hardware
    - Raspberry Pi 2 (You can try porting it to other platforms)
    - Car chassis
    - DC Motor
    - DC Motor Gear Box (this usually comes along with the car chasis)
    - Tyres 
    - Step-up regulator L298N 
    - Ultrasonic Sensor 
    - 18650 Battery x2
    - Battery Holder for 18650 batteries
Software
    - Raspberry Pi
        - V4L2
        - UV4L
        - make
        - g++ (compiling tools)
    - Remote Machine (Laptop/PC)
        - make
        - g++ (compiling tools)
        - openCV

