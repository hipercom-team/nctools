/*---------------------------------------------------------------------------
 * Linear coding of packets
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#ifndef __LINEAR_PACKET_H__
#define __LINEAR_PACKET_H__

/*---------------------------------------------------------------------------*/

#include <stdint.h>
#include <string.h>

#include "general.h"

/*---------------------------------------------------------------------------*/

#define CODED_PACKET_SIZE 128

#define LOG2_CODING_HEADER_SIZE 4
#define CODING_HEADER_SIZE (1<<LOG2_CODING_HEADER_SIZE)

#define GENERATION_NONE 0xffffu
#define COEF_BYTE_NONE 0xffffu

typedef struct {
  uint8_t  log2_nb_bit_coef; /* 0,1,2,3 [-> 1,2,4,8 bits for coefficients] */
  uint16_t generation;  /* for sanity check */

  uint16_t raw_coef_byte_start; 
  uint16_t raw_coef_byte_end;
  uint16_t data_size;

  union {
    uint32_t u32[(CODING_HEADER_SIZE+CODED_PACKET_SIZE+3)/4];
    //uint16_t u16[(CODING_HEADER_SIZE+CODED_PACKET_SIZE+1)/2];
    uint8_t  u8[CODING_HEADER_SIZE+CODED_PACKET_SIZE];
  } content;
} coded_packet_t;

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

#define MAX_CODED_PACKET (CODING_HEADER_SIZE * BITS_PER_BYTE)

struct s_packet_set_t;
typedef void(*packet_set_notify_decoded)(struct s_packet_set_t* packet_set,
					 uint16_t packet_index,
					 uint16_t packet_base);

#define PACKET_INDEX_NONE 0xffffu

typedef struct s_packet_set_t {
  coded_packet_t packet_table[MAX_CODED_PACKET];
  uint16_t index_of_base[MAX_CODED_PACKET];
  uint16_t base_of_index[MAX_CODED_PACKET];
} packet_set_t;

/*---------------------------------------------------------------------------*/

#endif /* __LINEAR_PACKET_H__ */
