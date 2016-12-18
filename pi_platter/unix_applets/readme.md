## Documentation

This directory contains Raspbian utility software for the Solar Pi Platter.

###talkpp
talkpp is a utility program to simplify communicating with the Solar Pi Platter.  It provides a simple command-line interface to allow the user to directly (or via scripts) send commands to the board and to easily manage the Real Time Clock.  It uses udev to automatically find the correct serial device for the board, independent of other USB serial devices.

Both the source and a binary compiled under Raspbian Jessie are included.  The binary can simply be downloaded and installed in /usr/local/bin.  The source is easily compiled

    gcc -o talkpp talkpp.c -ludev

talkpp takes the following arguments:

   talkpp [-c <command string>]

          [-s] [-t] [-f]

          [-a <alarm timespec>] [-d <delta seconds>] [-w]

          [-u | -h]


   -c <command string> : send the command string.  Command strings without an "=" character cause the utility to echo back a response.

   -s : Set the Device RTC with the current system clock

   -t : Get the time from the Device RTC and display it in a form useful to pass to "date" to set the system clock ("+%m%d%H%M%Y.%S")

   -f : Get the time from the Device RTC and display it in a readable form.

   -a <alarm timespec> : Set the Device wakeup value (does not enable the alarm).  <alarm timespec> is the alarm time in date time format ("+%m%d%H%M%Y.%S")

   -d <delta seconds> : Set the Device wakeup to <delta seconds> past the current Device RTC time value (does not enable the alarm)

   -w : Display the wakeup value in a readable form.

   -u, -h : Usage (and optional help)


Example command to Solar Pi Platter: `talkpp -c B`

Setting the Solar Pi Platter RTC from the Pi's RTC: `talkpp -s`

Setting the Pi's RTC from the Solar Pi Platter (using BASH): `sudo date $(talkpp -t)`

talkpp will echo responses from the Solar Pi Platter to stdout.  It will also echo the last warnings or error messages that has been sent.

###Questions?

Contact the author @ dan@danjuliodesigns.com

