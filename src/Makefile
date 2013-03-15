#---------------------------------------------------------------------------
# Author: Cedric Adjih
# Copyright 2013 Inria
# All rights reserved. Distributed only with permission.
#---------------------------------------------------------------------------

PYTHON=python
CFLAGS += -Wall

#------------------------------

SRCS = linear-operation.c linear-packet.c general.c

HEADERS = $(SRCS:.c=.h)

OBJS = $(SRCS:.c=.o)

#---------------------------------------------------------------------------
# Main targets
#---------------------------------------------------------------------------

all: libnc.a _libncmodule.so

#---------------------------------------------------------------------------
# Dependencies
# http://stackoverflow.com/questions/97338/gcc-dependency-generation-for-a-different-output-directory
#---------------------------------------------------------------------------

DEPS = $(SRCS:.c=.d)

include $(DEPS)

%.d : %.c
	$(CC) $(CCFLAGS) -MF"$@" -MG -MM -MP -MT"$@" -MT"$(<:.c=.o)" "$<"

#---------------------------------------------------------------------------
# Automated file generation
#---------------------------------------------------------------------------

table-gf%.pydat: computeGaloisFieldTable.py
	sage computeGaloisFieldTable.py $*

table-explog-gf%.c table-mul-gf%.c: writeGaloisFieldTable.py table-gf%.pydat
	python writeGaloisFieldTable.py $*

linear-operation.o: linear-operation.c \
   table-explog-gf256.c table-explog-gf16.c table-explog-gf4.c \
   table-mul-gf256.c table-mul-gf16.c table-mul-gf4.c

#---------------------------------------------------------------------------
# Library
#---------------------------------------------------------------------------

libnc.a: ${OBJS} Makefile
	${AR} cru $@ ${OBJS}

#--------------------------------------------------------------------------
# SWIG wrapper
#---------------------------------------------------------------------------

_libncmodule.so: libncmodule.c setup.py ${SRCS} ${OBJS}
	rm -rf build
	${PYTHON} setup.py build #|| python setup.py build
	ln -sf build/lib*/libncmodule.so $@ || true

libncmodule.c: libnc.i Makefile ${SRCS} ${HEADERS}
	swig -python -o $@ libnc.i

#---------------------------------------------------------------------------
# Testing
#---------------------------------------------------------------------------

unittest: all
	python testLibNC.py

#---------------------------------------------------------------------------
# Cleaning
#---------------------------------------------------------------------------

clean:
	rm -f *.a *.so *.o *.d *~ *.pyc 
	rm -f libncmodule.c libncmodule.py

really-clean: clean
	rm -rf build table-*.pydat table-*gf*.c

#---------------------------------------------------------------------------
