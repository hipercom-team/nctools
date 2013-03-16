/*---------------------------------------------------------------------------
 * Definitions specific for compiling on Linux
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#ifndef __PLATFORM_LINUX_H__
#define __PLATFORM_LINUX_H__

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
#include <unistd.h>

#define ASSERT( x ) assert ( x )

#define FATAL(...)				\
  BEGIN_MACRO					\
     fprintf(stderr, "FATAL: " __VA_ARGS__);	\
     fprintf(stderr, "\n");			\
     exit(1); \
  END_MACRO

#define WARN(...)				\
  BEGIN_MACRO					\
     fprintf(stderr, "WARNING: " __VA_ARGS__);	\
     fprintf(stderr, "\n");			\
  END_MACRO

/*---------------------------------------------------------------------------*/

#endif /* __PLATFORM_LINUX_H__ */
