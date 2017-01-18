## talkpp

This directory contains the code for the talkpp utilty.

talkpp is designed to run on a Raspberry Pi (or other linux computer) and allow communication with the Solar Pi Platter board either through the pseudo-tty created by the daemon ppd or the USB hardware port directly associated with the Solar Pi Platter if ppd is not running.  It provides a simple command line interface to send commands to (and get responses from) the Solar Pi Platter.  It also provides some commands to make managing time with the Solar Pi Platter's RTC easier.

It uses udev to be able to idenfity which hardware serial device belongs to the Solar Pi Platter (via the USB VID, PID and product strings).


###Building

A prebuilt binary is included here but building is very easy:

    gcc -o talkpp talkpp.c -ludev

The compiled binary may be copied to /usr/local/bin.

    sudo mv talkpp /usr/local/bin

You may have to set appropriate permissions (you will have to do this if you just copy the
prebuilt binary from git to /usr/local/bin).

    sudo chmod 775 /usr/local/bin/talkpp

libudev-dev must be installed:

    sudo apt-get update
    sudo apt-get install libudev-dev


###Usage

Type 'talkpp -u' or 'talkpp -h' for a list of command line options.

####Basic commands

ASCII command strings to be sent to the Solar Pi Platter using the '-c <string>' command.  For example to read the battery voltage:

    talkpp -c B

Or to set a servo position

    talkpp -c P1=100

talkpp will also echo the message (such as a WARN or ERR message) pending from the Solar Pi Platter when it connects.  It will also echo back any received responses for a command.


####Time management

The Solar Pi Platter RTC may be set from the Pi's clock (once it has been set) using the command.

    talkpp -s

The Pi clock may be set from the Solar Pi Platter RTC using a response in the linux 'date' command (for example when booting the Pi without any network connectivity).

    sudo date $(talkpp -t)

A human readable form of the Solar Pi Platter date can be read using the '-f' command.

    talkpp -f

An wakeup alarm time, using the linux time format ("+%m%d%H%M%Y.%S") can be set using

    talkpp -a <alarm timespec>

A wakeup time using a delta second value past the current RTC time value.

    talkpp -d <delta seconds>

Note that both methods of setting a wakeup time do not enable the wakeup alarm.  This must be done using the "C0=1" command.

    talkpp -c C0=1

Viewing the wakeup time currently set in the Solar Pi Platter in a readable form.

    talkpp -w


###Useful Shutdown Script

The following is a simple script that you can put in /usr/local/bin that will shut down the Pi and then power-down the entire system in a controlled fashion.  I name the script "powerdown".  You can cut&paste the following code into any editor and then write it out and copy it to /usr/local/bin with "sudo mv".  Be sure to set appropriate executable permissions with "sudo chmod 775 /usr/local/bin/powerdown".

    #!/bin/bash
    # powerdown script for Raspberry Pi and Solar Pi Platter
    #
    /usr/local/bin/talkpp -c O=30
    sudo shutdown now


###Questions?

Contact the author @ dan@danjuliodesigns.com

