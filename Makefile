TARGET = ydict
OBJS = main.o browse.o ctrl.o pg.o chinesedraw.o misc.o selectdict.o \
		switchmode.o ringkey.o controller.o

INCDIR = $(PSPSDK)/../include 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP YDICT ver0.2
PSP_EBOOT_ICON = ydicticon0.2.png
PSP_EBOOT_PIC1 = ydictpic2.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
