# If we ever get another shared library, move this to another makefile

ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
Q_CC      = @echo ' ' CC '    ' $@;
Q_LD      = @echo ' ' LD '    ' $@;
Q_AR      = @echo ' ' AR '    ' $@;
Q_HEX     = @echo ' ' HEX '   ' $@;
Q_EEP     = @echo ' ' EEP '   ' $@;
Q_LSS     = @echo ' ' LSS '   ' $@;
Q_SZ      = @echo ' ' SIZE '  ' $@;
Q_RM      = @echo ' ' CLEAN ' ' $(TARGET);
Q_OPENOCD = @echo ' ' OPENOCD $(TARGET);
Q_BUILD   = @echo ' ' BUILD ' ' $(TARGET)
endif
endif

TARGET = bbfec.so
OUTDIR = .
CC = gcc
LD = gcc
SZ = size

CFLAGS = -Wall -std=gnu99 -O2 -D_GNU_SOURCE -fPIC -Werror -ggdb

LDFLAGS = -shared -Wl,-soname,$(TARGET)

INCLUDES = -I.

LIBDIR = -L.
LIBS = -lpthread -lrt

SOURCES=viterbi.c rs.c randomizer.c

OBJECTS=$(SOURCES:.c=.o)

.PHONY: all clean

all: $(SOURCES) $(TARGET)

.c.o:
	$(Q_CC)$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS) $(ARCHIVES)
	$(Q_LD)$(LD) $(LDFLAGS) $(LIBS) -o $(TARGET) $(OBJECTS) 
	
size:
	$(Q_SZ)$(SZ) -t $(TARGET)

clean:
	-$(Q_RM)rm -rf $(OBJECTS) $(TARGET)
