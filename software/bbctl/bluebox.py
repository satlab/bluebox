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

	REQUEST_LEDCTL = 0x01

	LOOPBACK_OUT = (usb.util.ENDPOINT_OUT | 2)
	LOOPBACK_IN  = (usb.util.ENDPOINT_IN  | 1)
	
	def __init__(self):
		self.dev = None
		while self.dev is None:
			self.dev = usb.core.find(idVendor=self.VENDOR, idProduct=self.PRODUCT)
			time.sleep(0.1)

		if self.dev.is_kernel_driver_active(0) is True:
			self.dev.detach_kernel_driver(0)

		self.dev.set_configuration()

		self.manufacturer = usb.util.get_string(self.dev, 100, self.dev.iManufacturer)
		self.product = usb.util.get_string(self.dev, 100, self.dev.iProduct)
		self.serial = usb.util.get_string(self.dev, 100, self.dev.iSerialNumber)

	def led_get(self):
		bmRequestType = usb.util.build_request_type(
					usb.util.CTRL_IN,
					usb.util.CTRL_TYPE_CLASS,
					usb.util.CTRL_RECIPIENT_INTERFACE)
		return self.dev.ctrl_transfer(bmRequestType, self.REQUEST_LEDCTL, 0, 0, 1)[0]

	def led_set(self, enable):
		bmRequestType = usb.util.build_request_type(
					usb.util.CTRL_OUT, 
					usb.util.CTRL_TYPE_CLASS, 
					usb.util.CTRL_RECIPIENT_INTERFACE)
		self.dev.ctrl_transfer(bmRequestType, self.REQUEST_LEDCTL, 0, 0, [enable])

	def led_toggle(self):
		status = self.led_get()
		self.led_set(not status)

	def loopback_write(self, text):
		self.dev.write(self.LOOPBACK_OUT, text, 0)

	def loopback_read(self):
		return self.dev.read(self.LOOPBACK_IN, 64, 0)
