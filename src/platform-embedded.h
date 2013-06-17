/*---------------------------------------------------------------------------
 * Definitions specific for compiling on Linux
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#ifndef __PLATFORM_EMBEDDED_H__
#define __PLATFORM_EMBEDDED_H__

/*---------------------------------------------------------------------------*/

#include "config-embedded.h"

/*---------------------------------------------------------------------------*/

#define CONF_WITH_FPRINTF
#define fprintf(out, ...) printf(__VA_ARGS__)
#define FILE int

/*---------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/* see also: http://en.wikipedia.org/wiki/C_data_types (c99 data types) */
/** boolean type */
//typedef unsigned char bool_t;

/*---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define ASSERT( x ) do { if (!(x)) { for (;;) ; } } while(0)

#define FATAL(...)				\
  BEGIN_MACRO					\
     printf("FATAL: " __VA_ARGS__);	\
     printf("\n");			\
     while (1) ; \
  END_MACRO

#define WARN(...)				\
  BEGIN_MACRO					\
     printf("WARNING: " __VA_ARGS__);	\
     printf("\n");			\
  END_MACRO

/*---------------------------------------------------------------------------*/

#endif /* __PLATFORM_EMBEDDED_H__ */
