# Copyright (c) 2012 Jeppe Ledet-Pedersen <jlp@satlab.org>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import time
import usb

class Bluebox(object):
	VENDOR  = 0x1d50
	PRODUCT = 0x6666

	# LED Control
	REQUEST_LEDCTL		= 0x01

	# RF Control
	REQUEST_FREQUENCY 	= 0x02
	REQUEST_MODINDEX	= 0x03
	REQUEST_CSMARSSI	= 0x04
	REQUEST_POWER		= 0x05
	REQUEST_AFC		= 0x06
	REQUEST_IFBW		= 0x07
	REQUEST_TRAINING	= 0x08
	REQUEST_SYNCWORD	= 0x09
	REQUEST_TEST		= 0x0A
	REQUEST_REGISTER	= 0x0B

	# Data Control
	REQUEST_DATA		= 0x10

	# Bootloader Control
	REQUEST_BOOTLOADER	= 0xFF

	# Data Endpoints
	LOOPBACK_IN  = (usb.util.ENDPOINT_IN  | 1)
	LOOPBACK_OUT = (usb.util.ENDPOINT_OUT | 2)
	DATA_IN	     = (usb.util.ENDPOINT_IN  | 3)
	DATA_OUT     = (usb.util.ENDPOINT_OUT | 4)
	
	def __init__(self):
		self.dev = None
		while self.dev is None:
			self.dev = usb.core.find(idVendor=self.VENDOR, idProduct=self.PRODUCT)
			time.sleep(0.1)

		if self.dev.is_kernel_driver_active(0) is True:
			self.dev.detach_kernel_driver(0)

		self.dev.set_configuration()

		self.manufacturer = usb.util.get_string(self.dev, 100, self.dev.iManufacturer)
		self.product      = usb.util.get_string(self.dev, 100, self.dev.iProduct)
		self.serial       = usb.util.get_string(self.dev, 100, self.dev.iSerialNumber)

	def _ctrl_send(self, request, data, timeout=1000):
		bmRequestType = usb.util.build_request_type(
					usb.util.CTRL_OUT,
					usb.util.CTRL_TYPE_CLASS,
					usb.util.CTRL_RECIPIENT_INTERFACE)
		self.dev.ctrl_transfer(bmRequestType, request, 0, 0, data, timeout)

	def _ctrl_read(self, request, length, timeout=1000):
		bmRequestType = usb.util.build_request_type(
					usb.util.CTRL_IN,
					usb.util.CTRL_TYPE_CLASS,
					usb.util.CTRL_RECIPIENT_INTERFACE)
		return self.dev.ctrl_transfer(bmRequestType, request, 0, 0, length, timeout)

	def led_get(self):
		return self._ctrl_read(self.REQUEST_LEDCTL, 1)[0]

	def led_set(self, enable):
		self._ctrl_send(self.REQUEST_LEDCTL, [enable])

	def led_toggle(self):
		status = self.led_get()
		self.led_set(not status)

	def rf_frequency(self, freq):
		pass

	def rf_modindex(self, mi):
		pass

	def rf_power(self, dbm):
		pass

	def rf_ifbw(self, ifbw):
		pass

	def rf_syncword(self, word, tol):
		pass

	def rf_training(self, startms, interms):
		pass

	def rf_config(self):
		pass

	def rf_reg_read(self, reg, value):
		pass

	def rf_reg_write(self, reg):
		pass

	def bootloader(self):
		try:
			self._ctrl_send(self.REQUEST_BOOTLOADER, None, 0)
		except:
			pass

	def loopback_write(self, text):
		self.dev.write(self.LOOPBACK_OUT, text, 0)

	def loopback_read(self):
		return self.dev.read(self.LOOPBACK_IN, 64, 0)
