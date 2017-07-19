#!/bin/bash
#
# Shell script to implement a simple time-lapse camera using the Solar Pi Platter, a Pi
# Zero and camera module.  Place this script, along with talkpp and the powerdown script,
# in the /usr/local/bin directory.
#
# This script is designed for use on a Raspberry Pi Zero with a USB WiFi Dongle plugged into
# USB Port 2 (the bottom switched port).  This way it can save a little power by keeping
# WiFi disabled when it is operating as a timelapse camera.  It will also work fine with the
# Raspberry Pi Zero W (built-in WiFi) with a slight increase in power usage.
#
# It is designed to be executed from /etc/rc.local on bootup with the following lines added
# to rc.local:
#
#    # Run the photography script
#    /usr/local/bin/timelapse.sh &> /dev/null &
#    
# Utilizes an external switch connected between the Pi Platter's A1 input and groun (with
# a 10k-ohm pull-up resistor connected between A1 and 3.3V).  The switch selects the
# following functionality:
#   1. Switch Closed (A1 connected to ground) : Picture offload mode.  Enable power to
#      switched USB port 2 for a WiFi dongle and exit script to leave machine booted.
#   2. Switch Open (A1 pulled to 3.3V) : Timelapse mode.  Take a picture, schedule next
#      picture and power down.
#
# Perform the following setup one time to configure the system:
#   1. Set the Pi Platter's hardware RTC from the Pi (assuming the Pi's clock as been set
#      via nmtp during boot) using the command "talkpp -s".
#   2. Configure the Pi Platter's USB Port 2 to power up in the "off" state by default
#      using the command "E10=0" to disable the port in configuration eeprom.
#   3. Create a directory for the photos using "mkdir /usr/local/pictures".  If you are
#      running Raspbian you will have to use "sudo mkdir /usr/local/pictures".
#
# To start the timelapse camera, simply power up the system using the Pi Platter power button
# with the switch open.  This will boot the Pi, take the first picture and then schedule the
# next photograph and power down.
#
# To boot the Pi for remote access, power up the system using the Pi Platter power button
# with the switch closed.  This will boot the Pi, enable the WiFi dongle and exit the script
# with the Pi running.  Use the "powerdown" script to shut down and power off.
#  
# Released into the public domain with no warranty.  Dan Julio, dan@danjuliodesigns.com
#

# #########################
# USER CONFIGURATION VALUES
# #########################
#
# Times to start and stop each date are military format: HHMM
# They should be adjusted for the current daylight hours.
#
STARTOFDAY=0530
ENDOFDAY=2100

#
# Time (in seconds) between photos.  This should be larger than the entire time it takes
# to boot, take a picture and power down again (probably between 60-90 seconds, depending
# on the operating system).
#
TIMELAPSE=300




# #########################
# SCRIPT CODE
# #########################

# Execute based on the switch value
#   Values near 0 indicate a closed switch, values above around 700 indicate an open switch
SWITCHADC=$(talkpp -c A1)
if [ $SWITCHADC -le 100 ]; then
	# Enable USB Port 2
	talkpp -c U2=1
	# Disable any set alarms
	talkpp -c C0=0
	# Prematurely exit the script
	exit
fi

# Get the date from the RTC
MYDATE=$(talkpp -t)
MYHHMM=${MYDATE:4:4}

# Set our date from the RTC
date $MYDATE

# Set our next wakeup time.
if [ $MYHHMM -gt $ENDOFDAY ]; then
	# Getting dark: Set an alarm for tomorrow morning
	talkpp -a $(date --date=tomorrow +%m%d$STARTOFDAY%Y.00)
else
	# Set an alarm for the next picture today
	talkpp -d $TIMELAPSE
fi
# Enable the alarm
talkpp -c C0=1

# Get the current board status
STATUS=$(talkpp -c S)

# Collect power-related info and log
BATTV=$(talkpp -c B)
echo $MYDATE,$BATTV,$STATUS >> /usr/local/pictures/power_info.txt

# Take a picture
raspistill -n -t 1000 -w 1024 -h 768 -o /usr/local/pictures/$MYDATE.jpg

# And finally shutdown and then power off
talkpp -c O=15
shutdown now

