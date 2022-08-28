## ppd

This directory contains the code for the ppd daemon.

ppd is a daemon program designed to run on a Raspberry Pi (or other linux computer).  It allows the system to execute a controlled shutdown in the case of a critically low battery voltage (that has triggered the Solar Pi Platter to start a count-down to powering down the entire system).  This is important to protect the Raspberry Pi's SD-card filesystem from corruption that can occur when power is removed from a running system.  It does this by connecting to the hardware serial port associated with the Solar Pi Platter (like talkpp, it uses udev to figure this out) and looks for the WARN string sent the Solar Pi Platter for critical battery detection.  Since it "commandeers" the hardware serial port, it also provides additional mechanisms for user application code to communicate with the Solar Pi Platter.  It always creates the pseudo-tty "/dev/pi-platter" which can be treated like a serial port connection.  It can optionally also create a TCP socket for network communication with the Solar Pi Platter.

It uses udev to be able to idenfity which hardware serial device belongs to the Solar Pi Platter (via the USB VID, PID and product strings).


###Building

A prebuilt binary is included here but building is very easy:

    gcc -o ppd ppd.c -ludev

The compiled binary should be copied to /usr/local/bin.

    sudo mv ppd /usr/local/bin

You may have to set appropriate permissions (you will have to do this if you just copy the
prebuilt binary from git to /usr/local/bin).

    sudo chmod 775 /usr/local/bin/ppd

libudev-dev must be installed:

    sudo apt-get update
    sudo apt-get install libudev-dev

###Usage

ppd should be started as a daemon at start-up.  The easiest way to do this is to include somethine like the following in /etc/rc.local.

    # Start the Pi Platter Daemon
    /usr/local/bin/ppd -p 23000 -r -d &

It is also possible to create a conf file in /etc/init or a startup script in /etc/init.d.

####Command line options

ppd takes the following command line arguments:

    -d : Run as a daemon program (disconnecting from normal IO, etc).  ppd can be run as a traditional process without this argument.

    -p netport : Enable a TCP socket connection on the specified port.  This is required to enable socket communication with ppd.  Exclude this line to only enable /dev/pi-platter as a mechanism to communicate with the Solar Pi Platter.

    -m max-connections : Specify the maximum number of socket connections that can be made to the port specified with -p.  The default is 1.
    
    -o seconds : Configure Pi Platter to shut off power after the specified period upon receipt of SIGTERM.

    -r : Enable auto-restart on charge (set the Pi Platter "C7=1") after critical battery shutdown.

    -x debuglevel : Set the debug level (ppd uses the system logging facility.  0 is default (only log start-up).  Values of 1 - 3 include progressively more information.

    -h : Display usage and command line options.

####telnet example

To connect to the TCP socket using telnet from some other computer:

    telnet <Raspberry Pi IP Address> 23000

Which should result in something like (user types "B" and then "S"):

    Trying <Raspberry Pi IP Address>...
    Connected to raspberrypi0.lan.
    Escape character is '^]'.
    B
    B=4.12
    S
    S=20

Note that opening the socket without additional precautions may be a security risk (anybody could telnet to your Pi and shut it down).  That's why ppd makes this an option.

####Soft shutdown
The ```-o``` option (introduced in version 0.2) configures the program to automatically power off the Pi a configurable number of seconds after receiving a SIGTERM signal.  SIGTERM is sent when the Pi has been commanded to shut down (for example using the GUI or ```shutdown``` command).

Start the program as a daemon using this option with an argument of seconds longer than it takes for the Pi to execute a shutdown and the system will power off automatically.

	/usr/local/bin/ppd -p 23000 -o 30 -r -d &

Just be sure to use a SIGKILL to stop the daemon (```kill -9```) so that it doesn't misinterpret a ```kill`` you are sending it to stop it running for a system shutdown.

###Questions?

Contact the author @ dan@danjuliodesigns.com

