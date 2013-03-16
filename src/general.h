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

#include <stdint.h>

#include "libnc-config.h"

/*---------------------------------------------------------------------------*/

#define BEGIN_MACRO do {
#define END_MACRO } while(0)

#define DO_NOTHING BEGIN_MACRO END_MACRO

/*---------------------------------------------------------------------------*/

#define BITS_PER_BYTE 8
#define LOG2_BITS_PER_BYTE 3

#define BOOL_FALSE 0
#define BOOL_TRUE  1

#define REQUIRE(...) ASSERT(__VA_ARGS__)
#define ENSURE(...)  ASSERT(__VA_ARGS__)

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

static inline uint16_t min_except(uint16_t v1, uint16_t v2, uint16_t ignored)
{
  if (v1 == ignored) 
    return v2;
  if (v2 == ignored)
    return v1;
  return MIN(v1, v2);
}

static inline uint16_t max_except(uint16_t v1, uint16_t v2, uint16_t ignored)
{
  if (v1 == ignored) 
    return v2;
  if (v2 == ignored)
    return v1;
  return MAX(v1, v2);
}

/*---------------------------------------------------------------------------*/

#include "platform-linux.h"

/*---------------------------------------------------------------------------*/

#ifdef CONF_WITH_FPRINTF 

void data_string_pywrite(FILE* out, uint8_t* data, int data_size);

#endif /* CONF_WITH_FPRINTF */

/*---------------------------------------------------------------------------*/


#endif /* __GENERAL_H__ */
