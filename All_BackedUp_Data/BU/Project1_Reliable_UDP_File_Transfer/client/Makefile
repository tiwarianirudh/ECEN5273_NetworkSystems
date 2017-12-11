# ANIRUDH TIWARI
# PROGRAMMING ASSIGNMENT 1
# References for Makefile: 1. http://mrbook.org/blog/tutorials/make/ and 2. http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/code/example-1/Makefile



INCLUDE_DIRS =
LIB_DIRS =
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS=

HFILES=
CFILES= client.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	client

clean:
	-rm -f *.o *.d
	-rm -f client

client: client.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
