/*---------------------------------------------------------------------------
 * Linear coding of packets
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#include "linear-packet.h"

/*---------------------------------------------------------------------------*/

static inline unsigned int get_packet_window(uint8_t log2_nb_bit_coef)
{ return DIV_LOG2(CODING_HEADER_SIZE, log2_nb_bit_coef); }

static inline unsigned int gen_of_base_packet(unsigned int base_packet, 
					      uint8_t log2_nb_bit_coef)
{ return DIV_LOG2(base_packet, (LOG2_CODING_HEADER_SIZE+log2_nb_bit_coef)); }

static inline unsigned int coef_index_of_base_packet(unsigned int base_packet, 
						   uint8_t log2_nb_bit_coef)
{ return MOD_LOG2(base_packet, (LOG2_CODING_HEADER_SIZE+log2_nb_bit_coef)); }

/* 
   base_packet_index bits are subdivided as:
   <generation><index of byte>                 <index of coef inside byte>
               [LOG2_CODING_HEADER_SIZE/8 bits][(8/log2_nb_bit_coef) bits]
*/

uint8_t coded_packet_get_coef(coded_packet_t* pkt, 
			      unsigned int base_packet_index)
{   
  //unsigned int coef_per_byte = DIV_LOG2(BITS_PER_BYTE, pkg->log2_nb_bit_coef);
  unsigned int log2_coef_per_byte = LOG2_BITS_PER_BYTE - pkg->log2_nb_bit_coef;
  unsigned int coef_index_inside_byte 
    = MOD_LOG2(base_packet_index, log2_coef_per_byte);
  unsigned int raw_byte_pos = DIV_LOG2(base_packet_index, log2_coef_per_byte);
  unsigned int byte_pos = MOD_LOG2(raw_byte_pos, LOG2_CODING_HEADER_SIZE);
  unsigned int generation = DIV_LOG2(raw_byte_pos, LOG2_CODING_HEADER_SIZE);


  unsigned int bit_pos = MUL_LOG2(coef_index_inside_byte, 
				  pkt->log2_nb_bit_coef);

  unsigned int coef = MOD_LOG2( (pkt->content.u8[byte_pos] >> bit_pos),
				(1<<pkt->log2_nb_bit_coef) );
}

void coded_packet_set_coef(coded_packet_t* pkt, uint16_t base_packet_index,
			   uint8_t coef)
{
  unsigned int nb_bit_coef = 1<<pkt->log2_nb_bit_coef;
  unsigned int log2_coef_per_byte = LOG2_BITS_PER_BYTE - pkg->log2_nb_bit_coef;
  unsigned int coef_index_inside_byte 
    = MOD_LOG2(base_packet_index, log2_coef_per_byte);
  unsigned int raw_byte_pos = DIV_LOG2(base_packet_index, log2_coef_per_byte);
  unsigned int byte_pos = MOD_LOG2(raw_byte_pos, LOG2_CODING_HEADER_SIZE);
  unsigned int generation = DIV_LOG2(raw_byte_pos, LOG2_CODING_HEADER_SIZE);

  unsigned int coef_mask = MASK(nb_bit_coef);

  unsigned int bit_pos = MUL_LOG2(coef_index_inside_byte, 
				  pkt->log2_nb_bit_coef);

  ASSERT( coef <= coef_mask );

  pkt->content.u8[byte_pos] = 
    (pkt->content.u8[byte_pos]&(coef_mask << bit_pos))
    | (coef<<bit_pos);

  if (coef == 0)
    return;

  if (pkt->generation == GENERATION_NONE) {
    pkt->generation = generation;
    pkt->raw_coef_byte_start = raw_byte_pos;
    pkt->raw_coef_byte_end = raw_byte_pos;
  } else {
    if (raw_byte_pos < pkt->raw_coef_byte_start)
      pkt->raw_coef_byte_start = raw_byte_pos;
    if (raw_byte_pos > pkt->raw_coef_byte_end)
      pkt->raw_coef_byte_end = raw_byte_pos;
    ASSERT( pkt->raw_coef_byte_end - pkt->raw_coef_byte_start 
	    < CODING_HEADER_SIZE );
  }
}

void coded_packet_init(coded_packet_t* pkt, uint8_t log2_nb_bit_coef)
{
  ASSERT( log2_nb_bit_coef <= MAX_LOG2_NB_BIT_COEF );
  pkt->log2_nb_bit_coef = log2_nb_bit_coef;
  pkt->generation = GENERATION_NONE;
  pkt->raw_coef_byte_start = COEF_BYTE_NONE;
  pkt->raw_coef_byte_end = COEF_BYTE_NONE;
  pkt->data_size = 0;
  memset(pkt->content.u8, 0, CODING_HEADER_SIZE);
}


void coded_packet_init_from_base_packet
(coded_packet_t* pkt, uint8_t log2_nb_bit_coef, uint16_t base_index,
 uint8_t* data, uint8_t data_size)
{
  coded_packet_init(pkt, log2_nb_bit_coef);
  coded_packet_set_coef(pkt, base_index, 1);
  ASSERT( pkt->data_size < data_size );
  memcpy(pkt->content.u8 + CODING_HEADER_SIZE, data, data_size);
  pkt->data_size = data_size;
}



#if 0
void coded_packet_mul(coded_packet_t* pkt, uint16_t coef)
{
  ASSERT( is_valid_coef_size(pkt->coef_size) );
  ASSERT( coef < (1<<pkt->nb_bit_coef) );

  if (coef == 0) {
    memset(pkt->content.u8, 0, sizeof(pkt->content.u8));
    return;
  }
  int i;
  for (i=0; i<CODING_HEADER_SIZE; i++)
    
}
#endif


#if 0
bool_t coded_packet_is_decoded(coded_packet_t* packet)
{
}

void packet_set_add(packet_set_t* packet_set, coded_packet_t* packet)
{
  int i;
  
  min_packet_index;
  max_packet_index;
}
#endif
