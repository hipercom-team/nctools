/*---------------------------------------------------------------------------
 * Linearly coded packets
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#ifndef __CODED_PACKET_H__
#define __CODED_PACKET_H__

/*---------------------------------------------------------------------------*/

#include <stdint.h>
#include <string.h>

#include "general.h"
#include "linear-coding.h"

/*---------------------------------------------------------------------------*/

#define CODED_PACKET_SIZE 128

#define LOG2_COEF_HEADER_SIZE 4
#ifndef SWIG
#define COEF_HEADER_SIZE (1<<LOG2_COEF_HEADER_SIZE)
#else /* SWIG */
#define COEF_HEADER_SIZE 16
#endif /* SWIG */

#define COEF_INDEX_NONE 0xffffu

typedef struct {
  uint8_t  log2_nb_bit_coef; /* 0,1,2,3 [-> 1,2,4,8 bits for coefficients] */

  uint16_t coef_index_min; 
  uint16_t coef_index_max;
  uint16_t data_size;

  union {
    uint32_t u32[(COEF_HEADER_SIZE+CODED_PACKET_SIZE+3)/4];
    //uint16_t u16[(COEF_HEADER_SIZE+CODED_PACKET_SIZE+1)/2];
    uint8_t  u8[COEF_HEADER_SIZE+CODED_PACKET_SIZE];
  } content;
} coded_packet_t;

/*---------------------------------------------------------------------------*/

static inline uint16_t coded_packet_log2_window(coded_packet_t* pkt)
{
  uint8_t l = pkt->log2_nb_bit_coef;
  return LOG2_COEF_HEADER_SIZE+LOG2_BITS_PER_BYTE-l;
}

void coded_packet_init(coded_packet_t* pkt, uint8_t log2_nb_bit_coef);

void coded_packet_init_from_base_packet
(coded_packet_t* pkt, uint8_t log2_nb_bit_coef, uint16_t base_index,
 uint8_t* data, uint8_t data_size);

void coded_packet_copy_from(coded_packet_t* dst, coded_packet_t* src);

void coded_packet_set_coef(coded_packet_t* pkt, uint16_t coef_index,
			   uint8_t coef_value);

uint8_t coded_packet_get_coef(coded_packet_t* pkt, uint16_t coef_index);

bool coded_packet_adjust_min_max_coef(coded_packet_t* pkt);

static inline bool coded_packet_is_empty(coded_packet_t* pkt)
{ return !coded_packet_adjust_min_max_coef(pkt); }

static inline void coded_packet_to_mul(coded_packet_t* pkt, uint8_t coef)
{ lc_vector_mul(coef, pkt->content.u8, COEF_HEADER_SIZE+pkt->data_size,
		pkt->log2_nb_bit_coef, pkt->content.u8); }

void coded_packet_to_add(coded_packet_t* result,
			 coded_packet_t* p1,
			 coded_packet_t* p2);

void coded_packet_add_mult
(coded_packet_t* p1, uint8_t coef2, coded_packet_t* p2);

bool coded_packet_is_empty_safe(coded_packet_t* pkt);

#ifdef WITH_FPRINTF
void coded_packet_pywrite(FILE* out, coded_packet_t* p);
#endif /* WITH_FPRINTF */

static inline uint8_t* coded_packet_data(coded_packet_t* p)
{ return p->content.u8 + COEF_HEADER_SIZE; }

/* warning slow function, also may change p1, p2 by calling
   coded_packet_adjust_min_max_coef(...) */
bool coded_packet_is_similar(coded_packet_t* p1, coded_packet_t* p2);

/*---------------------------------------------------------------------------*/

#endif /* __CODED_PACKET_H__ */
