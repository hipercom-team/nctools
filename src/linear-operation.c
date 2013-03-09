/*---------------------------------------------------------------------------
 * Linear operations (GF(2), GF(4), GF(16) and GF(256))
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#include <string.h>

#include "general.h"
#include "linear-operation.h"

/*--------------------------------------------------*/

//#define WITH_GF256_MUL_TABLE

#include "table-mul-gf16.c"
#include "table-mul-gf4.c"

/*--------------------------------------------------*/

#ifdef WITH_GF256_MUL_TABLE

#include "table-mul-gf256.c"
static inline uint8_t gf256_mul(uint8_t a, uint8_t b)
{ return gf256_mul_table[a][b]; }

static inline uint8_t gf256_inv(uint8_t a)
{ return gf256_inv_table[a]; }

#else /* WITH_GF256_MUL_TABLE */

#include "table-explog-gf256.c"
/* compute a.b as exp[log[a]+log[b]] */
static inline uint8_t gf256_mul(uint8_t a, uint8_t b)
{
  if ( a == 0 || b == 0)
    return 0;

  uint16_t log_res = ((uint16_t)gf256_log_table[a])
    + ((uint16_t)gf256_log_table[b]);
  log_res = (log_res + (log_res >> 8)) & 0xff; /* mod 255 */
  return gf256_exp_table[log_res];
}

static inline uint8_t gf256_inv(uint8_t a)
{ return gf256_exp_table[255-gf256_log_table[a]]; }

#endif /* WITH_GF256_MUL_TABLE */

/*---------------------------------------------------------------------------*/

void lc_add(uint8_t* data1, uint16_t size1,
	    uint8_t* data2, uint16_t size2,
	    uint8_t* result, uint16_t* result_size)
{
  uint16_t i;
  uint16_t common_size = 0;

  if (size1 < size2) {
    common_size = size1;
    *result_size = size2;
    for (i = common_size; i<*result_size; i++)
      result[i] = data2[i];
  } else if (size1 > size2) {
    common_size = size2;
    *result_size = size1;
    for (i = common_size; i<*result_size; i++)
      result[i] = data1[i];
  }

  for (i=0; i<common_size; i++)
    result[i] = data1[i] ^ data2[i];
}

/*---------------------------------------------------------------------------*/

void lc_mul_gf256(uint8_t coef, uint8_t* data, uint16_t size,
		  uint8_t* result)
{ 
  if (coef == 0) {
    memset(result, 0, size);
    return;
  }

  uint16_t i;
  for (i=0; i<size; i++)
    result[i] = gf256_mul(coef, data[i]);
}

void lc_mul_gf16(uint8_t coef, uint8_t* data, uint16_t size,
		 uint8_t* result)
{
  ASSERT( coef < 16 );
  uint16_t i;
  for (i=0; i<size; i++)
    result[i] = gf16_mul_table[coef][data[i]];
}

void lc_mul_gf4(uint8_t coef, uint8_t* data, uint16_t size,
		uint8_t* result)
{
  ASSERT( coef < 4 );
  uint16_t i;
  for (i=0; i<size; i++)
    result[i] = gf4_mul_table[coef][data[i]];
}

void lc_mul_gf2(uint8_t coef, uint8_t* data, uint16_t size,
		uint8_t* result)
{
  ASSERT( coef < 2 );
  if (coef == 0) {
    memset(result, 0, size);
  } else {
    if (data != result)
      memcpy(result, data, size);
  }
}


void lc_mul(uint8_t coef, uint8_t* data, uint16_t size,
	    uint8_t* result, uint8_t log2_nb_bit_coef)
{
  ASSERT( log2_nb_bit_coef < 3 );
  switch(log2_nb_bit_coef) {
  case 0: lc_mul_gf2(coef, data, size, result); break;
  case 1: lc_mul_gf4(coef, data, size, result); break;
  case 2: lc_mul_gf16(coef, data, size, result); break;
  case 3: lc_mul_gf256(coef, data, size, result); break;
  default: FATAL("invalid log2_nb_bit_coef");
  }
}

/*---------------------------------------------------------------------------*/

uint8_t lc_mul_scalar(uint8_t x, uint8_t y, uint8_t log2_nb_bit_coef)
{
  ASSERT( log2_nb_bit_coef < 3 );
  switch(log2_nb_bit_coef) {
  case 0: ASSERT(x < 2 && y < 2); return x&y;
  case 1: ASSERT(x < 4 && y < 4); return gf4_mul_table[x][y];
  case 2: ASSERT(x < 16 && y <16); return gf16_mul_table[x][y];
  case 3: return gf256_mul(x,y);
  default: 
    FATAL("invalid log2_nb_bit_coef");
    return 0;
  }
}

/*---------------------------------------------------------------------------*/

uint8_t lc_inv_scalar(uint8_t x, uint8_t log2_nb_bit_coef)
{
  ASSERT( log2_nb_bit_coef < 3 );
  switch(log2_nb_bit_coef) {
  case 0: ASSERT(x < 2); return x;
  case 1: ASSERT(x < 4); return gf4_inv_table[x];
  case 2: ASSERT(x < 16); return gf16_inv_table[x];
  case 3: return gf256_inv(x);
  default: 
    FATAL("invalid log2_nb_bit_coef");
    return 0;
  }
}

/*---------------------------------------------------------------------------*/
