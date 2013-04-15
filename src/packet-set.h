/*---------------------------------------------------------------------------
 * Sets of linear combinations of packets: management and decoding
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#ifndef __PACKET_SET_H__
#define __PACKET_SET_H__

/*---------------------------------------------------------------------------*/

#include "coded-packet.h"

/*---------------------------------------------------------------------------*/

typedef struct {
  unsigned int non_reduction;
  unsigned int reduction_success;
  unsigned int reduction_failure;
  unsigned int coef_pos_too_low;
  unsigned int coef_pos_too_high;
  unsigned int elimination;
  unsigned int decoded;
} reduction_stat_t;

static inline void reduction_stat_init(reduction_stat_t* stat)
{ memset(stat, 0, sizeof(reduction_stat_t)); }

/*---------------------------------------------------------------------------*/

#ifdef CONF_MAX_CODED_PACKET
#define MAX_CODED_PACKET CONFIG_MAX_CODED_PACKET
#else  /* CONF_MAX_CODED_PACKET */
#define MAX_CODED_PACKET 128
#endif /* CONF_MAX_CODED_PACKET */

struct s_packet_set_t;

typedef void (*notify_packet_decoded_func_t) 
(struct s_packet_set_t* packet_set, uint16_t packet_index);

typedef void (*notify_set_full_func_t) 
(struct s_packet_set_t* packet_set, uint16_t required_min_coef_pos);

#define PACKET_ID_NONE 0xfffeu

typedef struct s_packet_set_t {
  coded_packet_t coded_packet[MAX_CODED_PACKET];
  uint16_t id_to_pos[MAX_CODED_PACKET];
  uint16_t pos_to_id[MAX_CODED_PACKET];

  void* notif_data;
  notify_packet_decoded_func_t notify_packet_decoded_func;
  notify_set_full_func_t notify_set_full_func;

  uint16_t coef_pos_min; 
  uint16_t coef_pos_max;
  uint8_t log2_nb_bit_coef;
} packet_set_t;


void packet_set_init(packet_set_t* set, uint8_t log2_nb_bit_coef,
		     notify_packet_decoded_func_t notify_packet_decoded_func,
		     notify_set_full_func_t notify_set_full_func,
		     void* notif_data);

/* 
   return the packet_id associated with the inserted packet,
   or PACKET_ID_NONE otherwise 
   pkt is (usually) modified by the function
*/
uint16_t packet_set_add(packet_set_t* set, coded_packet_t* pkt,
			reduction_stat_t* stat);

uint16_t packet_set_get_id_of_pos(packet_set_t* set, uint16_t coef_pos);

bool packet_set_is_empty(packet_set_t* set);

uint16_t packet_set_count(packet_set_t* set);

/*---------------------------------------------------------------------------*/

#endif /* __PACKET_SET_H__ */
