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

void lc_add(uint8_t* data1, uint16_t size1,
	    uint8_t* data2, uint16_t size2,
	    uint8_t* result, uint16_t* result_size);

void lc_mul(uint8_t coef, uint8_t* data, uint16_t size,
	    uint8_t* result, uint8_t log2_nb_bit_coef);

uint8_t lc_inv_scalar(uint8_t x, uint8_t log2_nb_bit_coef);

uint8_t lc_mul_scalar(uint8_t x, uint8_t y, uint8_t log2_nb_bit_coef);

/*---------------------------------------------------------------------------*/

#endif /* __LINEAR_OPERATION_H__ */
