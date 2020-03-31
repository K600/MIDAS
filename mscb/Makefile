########################################################
#  Makefile for msc executable under linux
#
#  If the environment variable MIDASSYS is defined,
#  the mxml.c, strlcpy, and musbstd.c files are
#  taken from there. Otherweise the files from the 
#  mxml/ and midas/ subdirectories below 
#  the mscb directory are used.
#
#
#  $Id$
########################################################

# get OS type from shell
OSTYPE = $(shell uname)

# flag for optional USB support
NEED_USB =

ifeq ($(OSTYPE),Darwin)
OSTYPE = darwin
endif

ifeq ($(OSTYPE),Linux)
OSTYPE = linux
endif

OUTNAME       = msc 
CC            = gcc
FLAGS         = -g -O0 -DHAVE_MRPC -Wall -Wuninitialized -Iinclude

STRLCPY       = strlcpy.o

ifeq ($(OSTYPE),linux)
FLAGS += -DOS_LINUX -fpic
LIBS = 
endif

ifeq ($(OSTYPE),darwin)
FLAGS += -DOS_DARWIN -DOS_LINUX -I/usr/local/include/
LIBS  = -framework IOKit -framework CoreFoundation -lobjc
endif

OBJECTS       = msc.o mscb.o mscbrpc.o cmdedit.o mxml.o strlcpy.o

ifdef NEED_USB
FLAGS += -DHAVE_USB -DHAVE_LIBUSB10
LIBS += -lusb-1.0
OBJECTS += musbstd.o
endif

all: $(OUTNAME)

$(OUTNAME): $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) -o $(OUTNAME) $(LIBS)

$(OBJECTS): %.o: src/%.c
	$(CC) $(FLAGS) -c $<

shared: $(OBJECTS)
	$(CC) -shared -o mscb.so $(OBJECTS) $(LIBS)

clean:
	rm *.o $(OUTNAME)

