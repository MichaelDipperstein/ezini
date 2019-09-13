############################################################################
# Makefile for ezini INI formatted file reader
############################################################################
CC = gcc
LD = gcc
CFLAGS = -I. -O3 -Wall -Wextra -pedantic -ansi -c
LDFLAGS = -O3 -o

# Treat NT and non-NT windows the same
ifeq ($(OS),Windows_NT)
	OS = Windows
endif

ifeq ($(OS),Windows)
	EXE = .exe
	DEL = del
else	#assume Linux/Unix
	EXE =
	DEL = rm -f
endif

TARGET = sample$(EXE) strtest$(EXE)

all:		$(TARGET)

sample$(EXE):	sample.o ezini.o
		$(LD) $^ $(LDFLAGS) $@

sample.o:		sample.c ezini.h
		$(CC) $(CFLAGS) $<

strtest$(EXE):	strtest.o ezini.o
		$(LD) $^ $(LDFLAGS) $@

strtest.o:		strtest.c ezini.h
		$(CC) $(CFLAGS) $<

ezini.o:	ezini.c ezini.h
		$(CC) $(CFLAGS) $<

docs:		doxygen.conf ezini.c ezini.h sample.c strtest.c
		rm -rf docs
		doxygen $<

clean:
		$(DEL) *.o
		$(DEL) $(TARGET)
