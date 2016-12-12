## talkpp

This directory contains the code for the talkpp utilty.

talkpp is designed to run a Raspberry Pi (or other linux computer) and allow communication with the Solar Pi Platter board via usb.  It provides a simple command line interface to send commands to (and get responses from) the Solar Pi Platter.  It also provides some commands to make managing time wit the Solar Pi Platter's RTC easier.

It uses udev to be able to idenfity which serial device belongs to the Solar Pi Platter (via the USB VID, PID and product strings).


###Building

A prebuilt binary is included here but building is very easy:

 gcc -o talkpp talkpp.c -ludev

The compiled binary may be copied to /usr/local/bin.


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


###Questions?

Contact the author @ dan@danjuliodesigns.com

