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
%pointer_functions(reduction_stat_t, reductionStat)

//---------------------------------------------------------------------------



%inline %{

#include "macro-pywrite.h"

  WRAP_PYWRITE(packet_set_pyrepr, packet_set_pywrite, packet_set_t*);
  WRAP_PYWRITE(reduction_stat_pyrepr, reduction_stat_pywrite, 
	       reduction_stat_t*);

  void* my_inc_ref(PyObject* pyObject)
  { Py_INCREF(pyObject); return pyObject; }

  void my_dec_ref(void* pyObject)
  { Py_DECREF((PyObject*)pyObject); }


  void packet_set_py_notify_packet_decoded
    (packet_set_t* set, uint16_t packet_id)
  { 
    PyObject* pyNotifyObj = (PyObject*) set->notif_data;
    PyObject* result = PyObject_CallMethod
      (pyNotifyObj, "notifyPacketDecoded", "I", (unsigned int) packet_id);
    if (result == NULL) {
      /* there was an exception */
    } else Py_DECREF(result);
  }
  
  const notify_packet_decoded_func_t py_callback_packet_decoded
    = packet_set_py_notify_packet_decoded;


  void packet_set_py_notify_set_full
    (packet_set_t* set, uint16_t required_min_coef_pos)
  { 
    PyObject* pyNotifyObj = (PyObject*) set->notif_data;
    PyObject* result = PyObject_CallMethod
      (pyNotifyObj, "notifyFull", "I", (unsigned int) required_min_coef_pos);
    if (result == NULL) {
      /* there was an exception */
    } else Py_DECREF(result);
  }
  
  const notify_set_full_func_t py_callback_set_full
    = packet_set_py_notify_set_full;


  void packet_set_pywrite_stdout(packet_set_t* set)
  { packet_set_pywrite(stdout, set); }

  const unsigned int macro_LOG2_COEF_HEADER_SIZE = LOG2_COEF_HEADER_SIZE;
  const unsigned int macro_COEF_HEADER_SIZE = COEF_HEADER_SIZE;
  const unsigned int macro_CODED_PACKET_SIZE = CODED_PACKET_SIZE;
  const unsigned int macro_COEF_POS_NONE = COEF_POS_NONE;
  const unsigned int macro_PACKET_ID_NONE = PACKET_ID_NONE;
  const unsigned int macro_MAX_CODED_PACKET = MAX_CODED_PACKET;

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
