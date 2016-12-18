## Documentation

This directory contains the components used for the Pi Car designed as an example during the kickstarter campaign.  It probably can't be used as-is but may give some ideas.

### Motor Control

The Pi Car was built on an old Sparkfun Ardubot base.  The Ardubot was designed to be controlled by an Arduino with only forward/reverse control at a fixed speed but contained a prototyping area.  It used a L293D driver with the Enable input tied high.  I modified the board so the Solar Pi Platter PWM signals drove the Enable input for speed control.  A pair of NPN transistor inverters allowed Pi GPIO signals to control motor direction.  The high PWM frequency is used for smooth motor control.

A LT1171 boost converter was used to step up the battery voltage to approximately 8.5 volts to provide power for the motors.

### Video

A Logitech USB camera coupled with mjpg-streamer handled up to 15 fps at 320x240 pixels for reasonable video streaming.  mjpg-streamer setup is not described here but is fairly straight forward.

A Sharp distance sensor was mounted above the camera and connected to a Solar Pi Platter analog input with Vref set to 4.096 volts.  A table in the control software maps the ADC value to distance.

### Other peripherals

A USB reading lamp made the perfect steampunk headlight and can be turned on and off using the Pi Platter’s per-port USB power switching.

An inexpensive INA169-based current sensor in series between the battery and the rest of the system and wired to the Pi Platter’s other analog input gives current draw.  The Pi Car idles at around 700 mA from a charged battery and can pull upwards of 1500-2000 mA when the motors are running and loaded.

Finally a Qi receiver module connects to the Pi Platter’s USB power input for wireless charging.

### Joystick control

An old analog joystick (2 10-kohm pots and a button) was connected to an Arduino Leonardo (DF Robot beetle) running a simple sketch that reported joystick and button information over a USB Serial link.

### Software

There are three software components.

1. pi_car.c - This is the software running on the Raspberry Pi.  It uses a single TCP port for communication with the control software running on the control computer.  It requires [WiringPi](http://wiringpi.com/) to be installed (for access to the GPIO pins).
2. pi_car_tv - A [xojo](http://www.xojo.com/) project that implements the main control program.  This project may be compiled for Windows, Mac OS X or Linux.  Xojo is a commercial product which requires a paid license to build a binary executable but the project can be compiled and run from within the IDE without any license.
3. read_joystick.ino - The arduino sketch reading the joystick.  Should be put in a folder called read_joystick.

