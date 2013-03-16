/*---------------------------------------------------------------------------
 * Linear coding of packets
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#include "packet-set.h"

/*---------------------------------------------------------------------------*/

void packet_set_init(packet_set_t* set, uint8_t log2_nb_bit_coef,
		     notify_decoded_func_t notify_func)
{
  uint16_t i;
  for (i=0; i<MAX_CODED_PACKET; i++) {
    set->id_to_pos[i] = PACKET_ID_NONE;
    set->pos_to_id[i] = PACKET_ID_NONE;
  }
  set->coef_pos_min = COEF_INDEX_NONE;
  set->coef_pos_max = COEF_INDEX_NONE;
  set->log2_nb_bit_coef = log2_nb_bit_coef;
}

uint16_t packet_get_id_of_coef_pos(packet_set_t* set, uint16_t coef_pos)
{
  if (set->coef_pos_min == COEF_INDEX_NONE) {
    ASSERT( set->coef_pos_max == COEF_INDEX_NONE );
    return PACKET_ID_NONE;
  }
  
  if (coef_pos > set->coef_pos_max)
    return PACKET_ID_NONE;
  if (coef_pos < set->coef_pos_min)
    return PACKET_ID_NONE;
  return set->pos_to_id[coef_pos % MAX_CODED_PACKET];
}

static uint16_t packet_set_reduce
(packet_set_t* set, coded_packet_t* pkt,
 uint16_t* result_non_reduction_count,
 uint16_t* result_reduction_success_count,
 uint16_t* result_reduction_fail_count)
{
  uint16_t result_coef_pos = COEF_INDEX_NONE;
  *result_non_reduction_count = 0;
  *result_reduction_success_count = 0;
  *result_reduction_fail_count = 0;

  REQUIRE( !coded_packet_is_empty_safe(pkt) );
  uint16_t coef_pos;
  for (coef_pos = pkt->coef_pos_min; coef_pos <= pkt->coef_pos_max;
       coef_pos ++) {
    uint8_t coef = coded_packet_get_coef(pkt, coef_pos);
    if (coef == 0)
      continue;
    uint16_t packet_id = coded_packet_get_id_of_coef_pos(set, coef_pos);
    if (packet_id == PACKET_ID_NONE) {
      result_coef_pos = coef_pos;
      (*result_non_reduction_count) ++;
      continue;
    }

    coded_packet_t* base_pkt = &set->coded_packet[packet_id];
    ASSERT( coded_packet_get_coef(base_pkt, coef_pos) == 1 );

    ASSERT( base_pkt.coef_index_min != COEF_INDEX_NONE );
    ASSERT( base_pkt.coef_index_max != COEF_INDEX_NONE );
    ASSERT( coded_packet.coef_index_min != COEF_INDEX_NONE );
    ASSERT( coded_packet.coef_index_max != COEF_INDEX_NONE );
    uint16_t coef_pos_min = MIN(coded_packet->coef_index_min,
				base_packet->coef_index_min);
    uint16_t coef_pos_max = MAX(coded_packet->coef_index_max,
				base_packet->coef_index_max);
    if (coef_pos_max-coef_pos_min >= (1<<coded_packet_log2_window(base_pkt))) {
      (*result_reduction_fail_count)++;
      continue;
    }

    /* reduce by coded_packet */
    (*result_reduction_success_count)++;
    coded_packet_add_mult(pkt, lc_neg(coef), base_pkt);

    bool is_empty = coded_packet_adjust_min_max_coef(pkt);
    if (is_empty) {
      /* note: it is possible that reduction_fail_count > 0 and still a empty
	 packet is obtained (e.g. second arrival of the same packet) */
      return COEF_INDEX_NONE;
    }
  }
  ENSURE( result_coef_pos != COEF_INDEX_NONE );
  return result_coef_pos;
}

uint16_t packet_set_add(packet_set_t* set, coded_packet_t* pkt)
{
  REQUIRE( set->log2_nb_bit_coef == pkt->log2_nb_bit_coef );

  bool is_empty = coded_packet_adjust_min_max_coef(pkt);
  if (is_empty) 
    return PACKET_ID_NONE;

  /* find available packet set */
  uint16_t i;
  for (i=0; i<MAX_CODED_PACKET; i++) {
    if (set->id_to_pos[i] == PACKET_INDEX_NONE) 
      break;
  }
  if (i == MAX_CODED_PACKET) {
    WARN("no room in packet_set_add");
    return PACKET_INDEX_NONE;
  }



  /* */
}

/*---------------------------------------------------------------------------*/
