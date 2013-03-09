/*---------------------------------------------------------------------------
 * Linear operations (GF(2), GF(4), GF(16) and GF(256))
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#ifndef __LINEAR_OPERATION_H__
#define __LINEAR_OPERATION_H__

/*---------------------------------------------------------------------------*/

#include <stdint.h>

/*---------------------------------------------------------------------------*/

#define MAX_LOG2_NB_BIT_COEF 3

uint8_t lc_inv(uint8_t x, uint8_t log2_nb_bit_coef);

uint8_t lc_mul(uint8_t x, uint8_t y, uint8_t log2_nb_bit_coef);

/*---------------------------------------------------------------------------*/

#define DIV_LOG2(value, log2_divisor) ((value)>>(log2_divisor))
#define MUL_LOG2(value, log2_mult) ((value)<<(log2_mult))
#define MASK(nb_bit) ((1 << (nb_bit))-1)
#define MOD_LOG2(value, log2_divisor) ((value)&MASK((log2_divisor)))

#define LOG2_BITS_PER_BYTE 3

void lc_vector_add(uint8_t* data1, uint16_t size1,
		   uint8_t* data2, uint16_t size2,
		   uint8_t* result, uint16_t* result_size);

void lc_vector_mul(uint8_t coef, uint8_t* data, uint16_t size,
		   uint8_t log2_nb_bit_coef, uint8_t* result);

void lc_vector_set(uint8_t* data, uint16_t size, uint8_t log2_nb_bit_coef,
		   uint16_t coef_index, uint8_t coef_value);

uint8_t lc_vector_get(uint8_t* data, uint16_t size, uint8_t log2_nb_bit_coef,
		      uint16_t coef_index);

/*---------------------------------------------------------------------------*/

#endif /* __LINEAR_OPERATION_H__ */
