## Documentation

This directory contains Raspbian utility software for the Solar Pi Platter.

### talkpp
talkpp is a utility program to simplify communicating with the Solar Pi Platter.  It provides a simple command-line interface to allow the user to directly (or via scripts) send commands to the board and to easily manage the Real Time Clock.  

Starting with version 0.5 it can communicate with the Solar Pi Platter via either the pseudo-tty /dev/pi-platter if the ppd daemon is running or the actual hardware serial device associated with the board if ppd is not running.  It uses udev to automatically find the correct serial device for the board, independent of other USB serial devices.

Both the source and a binary compiled under Raspbian Jessie are included.  The binary can simply be downloaded and installed in /usr/local/bin.  The source is easily compiled in the directory containing the source file.

    sudo apt-get update
    sudo apt-get install libudev-dev
    gcc -o talkpp talkpp.c -ludev
    sudo mv talkpp /usr/local/bin

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

### ppd
ppd is a daemon for the Solar Pi Platter.  It provides two main functions.  It will execute a controlled shutdown if the Solar Pi Platter detects a critical battery voltage (and will power-down the entire system after [default] 30 seconds).  Since it opens the serial port associated with the Solar Pi Platter it also provides one or two mechanisms for other applications to communicate with the Solar Pi Platter.  It creates a pseudo-tty device named /dev/pi-platter which can be used just like the hardware serial port.  It also, optionally, can create a TCP port for applications like telnet to connect to.  

It is important that software not open the hardware serial port, /dev/ttyACM<n>, when ppd is running since it is using the port.

ppd takes the following command line arguments:

    -d : Run as a daemon program (disconnecting from normal IO, etc).  ppd can be run as a traditional process without this argument.

    -p netport : Enable a TCP socket connection on the specified port.  This is required to enable socket communication with ppd.  Exclude this line to only enable /dev/pi-platter as a mechanism to communicate with the Solar Pi Platter.

    -m max-connections : Specify the maximum number of socket connections that can be made to the port specified with -p.  The default is 1.

    -o seconds : Configure Pi Platter to shut off power after the specified period upon receipt of SIGTERM.

    -r : Enable auto-restart on charge (set the Pi Platter "C7=1") after critical battery shutdown.

    -x debuglevel : Set the debug level (ppd uses the system logging facility.  0 is default (only log start-up).  Values of 1 - 3 include progressively more information.

    -h : Display usage and command line options.

There are many ways to start a daemon, for example a configuration file in / or a script in /etc/init.d.  A very easy way to start it is to include it in /etc/rc.local.  For example, add the following before the "exit 0" at the end of /etc/rc.local (assuming you have placed the ppd executable in /usr/local/bin).

    # Start the Pi Platter Daemon
    /usr/local/bin/ppd -p 23000 -r -d &

This starts ppd with socket communication available on port 23000 and auto-restart in the event of a critical battery shutdown.

Both the source and a binary compiled under Raspbian Jessie are included.  The binary can simply be downloaded and installed in /usr/local/bin.  The source is easily compiled in the directory containing the source file.

    sudo apt-get update
    sudo apt-get install libudev-dev
    gcc -o ppd ppd.c -ludev
    sudo mv ppd /usr/local/bin

### pp\_power\_mon
pp\_power\_mon is a Desktop widget designed to display the Solar Pi Platter voltage and charge status as well as provide control over the two switchable USB ports.


