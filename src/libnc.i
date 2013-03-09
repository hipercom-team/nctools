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
%array_class(unsigned char, byteArray)

%{
#include "linear-operation.h"
#include "linear-operation.c"
%}

%include "linear-operation.h"

//---------------------------------------------------------------------------
