Object Detect with SIFT on Autonomous Vehicle using Raspberry Pi
================================================================

This is a program that runs on Raspberry Pi. The main goal of this program is to detect object based on SIFT. All complex computations which includes control of vehicle and SIFT are done remotely, which means that Raspberry Pi only acts as a system that receives commands from a remote computation unit to control our vehicle. 

SIFT is an algorithm that's can point out the location of an object in an image by using a bounding box wrapping the object. The matching is based on features extracted from SIFT. This version of SIFT is implemented by ourselves so it's relatively slower than the libraries provided by OpenCV, but based on our experiment (we are using Intel(R) Core(TM) i5-3320M CPU), it doesn't seem to be a problem in the tracking of an object by our vehicle.

The connection protocol between Raspberry Pi and our remote machine will be through WiFi. We've are able to achieve real-time detection using 640x480 frames, so there'll be no issues with the real-time detection of images. (High resolution might result in laggy response in object detection)

Using this program, you'll be able to:
  - Control your vehicle using your keyboard (W,A,S,D)
  - Real-time detection of object(image) fed into this program

Requirements
------------
Hardware
  - Raspberry Pi 2 (You can try porting it to other platforms)
  - Router (for WiFi connection)
  - WiFi Dongle (For Raspberry Pi, we're using Asus N10)
  - USB Camera (Logitech C920)
  - Car chassis
  - DC Motor
  - DC Motor Gear Box (this usually comes along with the car chassis)
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

Usage
-----
A Makefile is written, thus to compile we've to run:
    make

After compiling, an excutable file name **controller** will be generated.

The program can be used in two modes which are single object detection and multi object detection. 

For single object detection, we've to name the target to be detected as **target.jpg** or a simple softlink will do. The image must be in the same directory as the execuatable for it to work.

For mutli object detection, all pictures that needs to be used as target will be placed into the **target** directory. The number of target varies with the resolution of the target and the resolution of your screen. Having 3 targets would usually occupy your whole screen, thus having more targets than that is pretty useless. *The name of the file doesn't matter.*

Once all images to be detected are ready, we can run the executable that we just compiled.

For single object detection, the following command would suffice:
    ./controller

For multi object detection, we've to run:
    ./controller multi

When the program is running, there're few operations that we can do.

- Control Vehicle: W,A,S,D
- Take Picture: P
- Switch to Next Target: N (only works for multi object detection)
- Shutdown Raspberry Pi: H
- Terminate Program: C
- Help Dialog: H or any key not in the list 


![Alt text](target/jijijin.jpg?raw=true "Optional Title")
