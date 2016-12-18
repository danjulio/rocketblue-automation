#!/bin/bash
#
# Shell script designed to take 3 pictures at a time in 3 different directions
# using a Pi Camera mounted on top of a servo connected to PWM 1.  Designed for
# use in a remote location with a Solar Panel charging the battery.
#
# The Solar Pi Platter wakes the Pi up every 15 minutes during the day and moves the
# servo to one of three positions, taking a picture and storing the picture in the
# Documenents directory.
#
# The script is designed to be run by /etc/rc.local when the Pi boots.  It looks at
# the power-up reason and does not execute if the Pi was powered on because the user
# powered up using the Solar Pi Platter button (to offload the images, for example).  
#

# Get the date from the RTC
MYDATE=$(talkpp -t)
MYHHMM=${MYDATE:4:4}

# Set our date from the RTC
date $MYDATE

# Get the power-up reason from the board (along with other status)
STATUS=$(talkpp -c S)

# Bail out of the script if we powered on because the user manually turned us on
if [ $STATUS -ge 16 ]; then
	# STATUS includes a power-up reason of "Button"
	exit
fi

# Set our next wakeup time
if [ $MYHHMM -gt 2000 ]; then
	# Getting dark: Set an alarm for tomorrow morning
	talkpp -a $(date --date=tomorrow +%m%d)06002016.00
	ENDOFDAY=1
else
	# Set an alarm for 15 minutes from now
	talkpp -d 900
	ENDOFDAY=0
fi
# Enable the alarm
talkpp -c C0=1

# Enable the servo
talkpp -c C6=3

# Move to position 1 and take a picture
talkpp -c P1=40
sleep 1
raspistill -n -t 100 -w 1024 -h 768 -o /home/pi/Documents/1_$MYDATE.jpg
#fswebcam -r 640x480 --no-banner /home/pi/Documents/1_$MYDATE.jpg

# Move to position 2 and take a picture
talkpp -c P1=128
sleep 1
raspistill -n -t 100 -w 1024 -h 768 -o /home/pi/Documents/2_$MYDATE.jpg
#fswebcam -r 640x480 --no-banner /home/pi/Documents/2_$MYDATE.jpg

# Move to position 3 and take a picture
talkpp -c P1=216
sleep 1
raspistill -n -t 100 -w 1024 -h 768 -o /home/pi/Documents/3_$MYDATE.jpg
#fswebcam -r 640x480 --no-banner /home/pi/Documents/3_$MYDATE.jpg

# At the end of the day, we remain powered for a bit in case someone wants to log in
# and download some images. 
if [ $ENDOFDAY -eq 1 ]; then
	sleep 1800
fi

# And finally shutdown and then power off
talkpp -c O=30
shutdown now

