/*
  DRAGONCAST
 */

#ifndef __DRAGONCAST_H__
#define __DRAGONCAST_H__

/*---------------------------------------------------------------------------*/

#include "platform-embedded.h"
#include "coded-packet.h"
#include "packet-set.h"

/*---------------------------------------------------------------------------*/

#define L 1
typedef uint8_t bool_t;

void dg_init(bool_t is_source);

void dg_process_coded_packet(uint8_t* packet_data, uint16_t packet_size);

uint16_t dg_generate_coded_packet
(uint8_t* packet_data, uint16_t max_packet_size);

int long dg_get_next_wakeup_time();

/*---------------------------------------------------------------------------*/

#endif /* __DRAGONCAST_H__ */
