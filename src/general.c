/*---------------------------------------------------------------------------
 * General definitions and macros
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#include "general.h"

/*---------------------------------------------------------------------------*/

#ifdef WITH_FPRINTF

void data_string_pywrite(FILE* out, uint8_t* data, int data_size)
{
  int i;
  fprintf(out, "'");
  for (i=0; i<data_size; i++)
    printf("\\x%02x", data[i]);
  fprintf(out, "'");
}

#endif /* WITH_FPRINTF */

/*---------------------------------------------------------------------------*/
