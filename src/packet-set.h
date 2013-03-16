/*---------------------------------------------------------------------------
 * Linearly coded packets
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

#define MAX_CODED_PACKET 128

struct s_packet_set_t;

typedef void (*notify_decoded_func_t) 
(struct s_packet_set_t* packet_set, uint16_t packet_index);

#define PACKET_ID_NONE 0xffffu

typedef struct s_packet_set_t {
  coded_packet_t coded_packet[MAX_CODED_PACKET];
  uint16_t id_to_pos[MAX_CODED_PACKET];
  uint16_t pos_to_id[MAX_CODED_PACKET];
  notify_decoded_func_t notify_decoded_func;

  uint16_t coef_pos_min; 
  uint16_t coef_pos_max;
  uint8_t log2_nb_bit_coef;
} packet_set_t;

void packet_set_init(packet_set_t* set, notify_decoded_func notify_func);

uint16_t packet_set_add(packet_set_t* set, coded_packet_t* pkt);

/*---------------------------------------------------------------------------*/

#endif /* __PACKET_SET_H__ */
