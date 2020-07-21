#---------------------------------------------------------------
# 
#  Makefile for Lanemark Detection
#
#  Ver. 1.1c
#
#  Use with bt848 0.3 driver.
#
#  Juliano Pimentel 2000 
#  https://github.com/julianop99
#
#---------------------------------------------------------------


CC = gcc
OFLAGS = -O -g -DUNIX -DLINUX

X11D = /usr/X11R6
BIND = ./
SRCD = src/
OBJD = obj/
INCD = -I$(X11D)/include/ -Iinclude/
LIBD = -L$(X11D)/lib/ -Llib/

LIBS = -lm -lc -lXm -lXt -lX11 -lXext -lXpm -lgrabber -lxdispima 

CFLAGS = $(OFLAGS) $(INCD)

#
# rules
#

all: lanemarkdetector


$(OBJD)lanemarkdetection.o: $(SRCD)LanemarkDetection.c $(SRCD)CameraViewApp.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJD)callbacks.o: $(SRCD)callbacks.c
	$(CC) -c $(CFLAGS) $< -o $@


OBJS = $(OBJD)lanemarkdetection.o $(OBJD)callbacks.o


$(BIND)lanemarkdetector: $(OBJS) 
	$(CC) -o $@ $^ $(LIBD) $(LIBS)



clean:
	-rm -f $(OBJD)* *~ $(SRCD)*~
	-rm -f $(BIND)lanemarkdetector $(BIND)*.pgm $(BIND)*.ppm


