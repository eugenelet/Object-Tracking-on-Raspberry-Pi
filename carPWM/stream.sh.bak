#!/bin/sh

mjpg_streamer -i "/usr/lib/input_uvc.so -d /dev/video0 -y -r 640x480 -f 10" -o "/usr/lib/output_http.so -p 8080 -w /var/www/mjpg_streamer" &
sudo ./car
