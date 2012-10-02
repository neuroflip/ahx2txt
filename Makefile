# make the printenv command
#
CFLAGS= -O
LDFLAGS= -s
CC=gcc
SRCS=ahx2txt.c
OBJS=ahx2txt.o
SHAR=shar

SYSTEMC = /usr/local/systemc-2.2
INCDIR = -I. -I..

all:    ahx2txt

# To get things out of the revision control system
$(SRCS):
	$(CC) $(CFLAGS) $(INCDIR) -c $*.c

# To make an executable

ahx2txt: $(OBJS)
	$(CC) $(LDFLAGS) $(INCDIR) -o $@ $(OBJS)

# clean out the dross
clean:
	rm ahx2txt *~ *.o 

