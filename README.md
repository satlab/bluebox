About the BlueBox
-----------------

The bluebox is a USB-powered ground station box design for the AAUSAT3 cubesat
satellite. The device firmware, control software and hardware is licensed under
the MIT license with the hope that it may be useful to other cubesat projects.

The device is built around the Analog Devices ADF7021 transceiver and controlled
by an Atmel ATMEGA32U4 microcontroller with hardware USB. The firmware makes use
of the open source LUFA USB stack and uses libusb and pyusb for communication
between the device and control program, bbctl.


How to install bbctl
--------------------

Go to software/bbctl and run:

	$ sudo python2 setup.py install

Note that you need gcc to compile the FEC extension module.

Requirements
------------

bbctl uses pyusb1.0, which is not yet available in sone linux distributions.

Follow these easy steps to install from source:

	$ git clone https://github.com/walac/pyusb.git
	$ cd pyusb
	$ sudo python2 setup.py install

How to build and flash the firmware
-----------------------------------

In order to communicate with device without being root, copy 49-bluebox.rules
and 49-atmel.rules to your udev rules directory (usually /etc/udev/rules.d).

Now, compile and flash the software. The bbctl dfu step can be omitted if
you're flashing the device for the first time:

	$ cd bluebox/software/firmware
	$ make clean all 
	$ bbctl dfu
	$ make program

You need dfu-programmer to program the device. Install it from your package
manager or from http://dfu-programmer.sourceforge.net/

You will also need a recent avr-8 compiler. Install it from your package
manager or from http://atmel.com

Finally, test the device by reading back the silicon revision of the ADF7021 RF
chip:

	$ bbctl version
	0x2104

You can run bbctl help to get a list of available commands.
