/*---------------------------------------------------------------------------
 * General definitions and macros
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#ifndef __GENERAL_H__
#define __GENERAL_H__

/*---------------------------------------------------------------------------*/

#define BEGIN_MACRO do {
#define END_MACRO } while(0)

#define DO_NOTHING BEGIN_MACRO END_MACRO

/*---------------------------------------------------------------------------*/

#define BITS_PER_BYTE 8

#define BOOL_FALSE 0
#define BOOL_TRUE  1

/*---------------------------------------------------------------------------*/

#include "platform-linux.h"

/*---------------------------------------------------------------------------*/

#endif /* __GENERAL_H__ */
