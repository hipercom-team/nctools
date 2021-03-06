#---------------------------------------------------------------------------
# Author: Cedric Adjih
# Copyright 2013 Inria
# All rights reserved. Distributed only with permission.
#---------------------------------------------------------------------------

#export CONTIKI_NO_NET=1

#--------------------------------------------------
# Contiki parts
#--------------------------------------------------

-include Makefile.target

ifndef TARGET
TARGET=z1
endif

#CONTIKI=contiki-senslab-unified
CONTIKI=contiki-standard
CONTIKI2=contiki-senslab-unified

CONTIKI_PROJECT = contiki-nc
#CONTIKI_PROJECT = test-adxl345
ifdef WITH_SHELL
APPS=serial-shell
CFLAGS += -DWITH_SHELL=1
endif
CONTIKI_SOURCEFILES += contiki-nc.c \
   general.c linear-code.c coded-packet.c packet-set.c

all: ${CONTIKI_PROJECT}
-include ${CONTIKI}/Makefile.include

CFLAGS += -DHIP_CONF_SIMULATOR=1 -DHIP_CONF_DISABLE_SWAP=1

#------------------------------

PYTHON=python
CFLAGS += -Wall -g3

#------------------------------

SRCS =  general.c linear-code.c coded-packet.c packet-set.c

HEADERS = $(SRCS:.c=.h)

OBJS = $(SRCS:.c=.o)

#------------------------------

DC_SRCS = dragoncast2008.c dragoncast2008.h

DC_HEADERS = $(DC_SRCS:.c=.h)

DC_OBJS = $(DC_SRCS:.c=.o)


#---------------------------------------------------------------------------
# Main targets
#---------------------------------------------------------------------------

swig-all: libdc2008.a libnc.a _libncmodule.so test-coded-packet

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

libnc.a: ${OBJS}
	${AR} cru $@ ${OBJS}

libdc2008.a: ${DC_OBJS}
	${AR} cru $@ ${DC_OBJS}

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

test-coded-packet: test-coded-packet.o libnc.a
	${CC} ${CFLAGS} -o $@ $< -lnc -L.

#---------------------------------------------------------------------------
# Cleaning
#---------------------------------------------------------------------------

clean-it: clean
	rm -f *.a *.so *.o *.d *~ *.pyc 
	rm -f libncmodule.c libncmodule.py
	rm -f test-coded-packet

really-clean: clean-it
	rm -rf build table-*.pydat table-*gf*.c 

#---------------------------------------------------------------------------
# COOJA Directory
#---------------------------------------------------------------------------

COOJADIR=${CONTIKI2}/tools/cooja

test-nogui:
	cd tmp && (cd ../${COOJADIR} ; ant jar) && java -jar ../${COOJADIR}/dist/cooja.jar -nogui=../sim-nc.csc  -contiki=../${CONTIKI2} 
# -external_tools_config=
# -quickstart=

run-nogui:
	cd tmp && java -jar ../${COOJADIR}/dist/cooja.jar -nogui=../sim-nc.csc  -contiki=../${CONTIKI2} #TEST-ARGS

run-gui:
	cd tmp && java -jar ../${COOJADIR}/dist/cooja.jar -quickstart=../sim-nc.csc  -contiki=../${CONTIKI2} #TEST-ARGS

run-cooja:
	cd ${COOJADIR} && ant 

#---------------------------------------------------------------------------
