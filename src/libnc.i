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

 //%array_functions(uint8_t, u8array)
 //%array_functions(uint16_t, u16array)
%array_class(uint8_t, u8array)
%array_class(uint16_t, u16array)
%pointer_functions(uint8_t,  u8ptr)
%pointer_functions(uint16_t, u16ptr)

//---------------------------------------------------------------------------

%{
#include "linear-operation.h"
#include "linear-operation.c"
%}

%include "linear-operation.h"

//---------------------------------------------------------------------------

%inline %{
  
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

%}

//---------------------------------------------------------------------------
