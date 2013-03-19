/*---------------------------------------------------------------------------
 * General definitions and macros
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#include "general.h"

/*---------------------------------------------------------------------------*/

#ifdef CONF_WITH_FPRINTF

/* write a data block as a python string with hex escape, 
   e.g. {1,0x40,0xff} -> '\x01\x40\xff' */
void data_string_pywrite(FILE* out, uint8_t* data, int data_size)
{
  int i;
  fprintf(out, "'");
  for (i=0; i<data_size; i++)
    fprintf(out, "\\x%02x", data[i]);
  fprintf(out, "'");
}

#endif /* CONF_WITH_FPRINTF */

/*---------------------------------------------------------------------------*/
