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

ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
Q_CC      = @echo ' ' CC '    ' $@;
Q_LD      = @echo ' ' LD '    ' $@;
Q_RM      = @echo ' ' CLEAN ' ' $(TARGET);
endif
endif

TARGET = ../bbfec.so
CC = gcc
LD = gcc
SZ = size

CFLAGS = -Wall -std=gnu99 -O2 -fPIC
LDFLAGS = -shared -Wl,-soname,$(TARGET)

SOURCES = viterbi.c rs.c randomizer.c
OBJECTS=$(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET) $(SOURCES)

.c.o:
	$(Q_CC)$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(Q_LD)$(LD) $(LDFLAGS) -o $@ $?
	
clean:
	$(Q_RM)rm -rf $(OBJECTS) $(TARGET)
