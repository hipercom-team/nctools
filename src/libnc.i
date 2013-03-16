//-----------------------------------------------------------------*- c++ -*-
// SWIG wrapper for libnc
//---------------------------------------------------------------------------
// Author: Cedric Adjih
// Copyright 2013 Inria
// All rights reserved. Distributed only with permission.
//---------------------------------------------------------------------------

%module libncmodule

%include "stdint.i"
%include "cdata.i"
%include "carrays.i"
%include "typemaps.i"
%include "cpointer.i"

%array_class(uint8_t, u8array)
%array_class(uint16_t, u16array)
%pointer_functions(uint8_t,  u8ptr)
%pointer_functions(uint16_t, u16ptr)
%array_functions(uint8_t, u8block)

//---------------------------------------------------------------------------

%{
#include "linear-code.h"
#include "linear-code.c"

#include "coded-packet.h"
#include "coded-packet.c"

#include "packet-set.h"
#include "packet-set.c"

#include "general.c"
%}

%include "linear-code.h"
%include "coded-packet.h"
%include "packet-set.h"
%include "macro-pywrite.h"

%pointer_functions(coded_packet_t, codedPacket)
%pointer_functions(packet_set_t, packetSet)

//---------------------------------------------------------------------------



%inline %{

#include "macro-pywrite.h"

  WRAP_PYWRITE(packet_set_pyrepr, packet_set_pywrite, packet_set_t*);
  
  const unsigned int macro_LOG2_COEF_HEADER_SIZE = LOG2_COEF_HEADER_SIZE;
  const unsigned int macro_COEF_HEADER_SIZE = COEF_HEADER_SIZE;
  const unsigned int macro_CODED_PACKET_SIZE = CODED_PACKET_SIZE;
  const unsigned int macro_COEF_POS_NONE = COEF_POS_NONE;

  void u8array_set(uint8_t* data, uint8_t value, int size)
  { memset(data, value, size); }

  void u8array_copy(uint8_t* data1, uint8_t* data2, int size)
  { memcpy(data1, data2, size); }

  int u8array_count_byte_diff(uint8_t* data1, uint8_t* data2, int size)
  {
    int i;
    int count = 0;
    for (i=0; i<size; i++)
      if (data1[i] != data2[i])
	count++;
    return count;
  }

  int u8array_count_bit_diff(uint8_t* data1, uint8_t* data2, int size)
  {
    int i;
    int count = 0;
    for (i=0; i<size; i++) {
      if (data1[i] != data2[i]) {
	uint8_t diff = data1[i] ^ data2[i];
	int j;
	for (j=0;j<8;j++)
	  if ((diff & (1<<j)) != 0)
	    count++;
      }
    }
    return count;
  }

  uint8_t* cast_to_u8ptr(char* data)
  { return (uint8_t*)data; }

%}

//---------------------------------------------------------------------------
