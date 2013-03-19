/*---------------------------------------------------------------------------
 * Test linear coding of packets
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "coded-packet.h"

/*---------------------------------------------------------------------------*/

#define L 3

#define COEF_PER_HEADER ((COEF_HEADER_SIZE*BITS_PER_BYTE)>>L)
coded_packet_t packet_table[COEF_PER_HEADER * 2];

int main(int argc, char** argv)
{
  int i;
  for (i=0; i<COEF_PER_HEADER*2; i++) {
    coded_packet_init(&packet_table[i], L);
    coded_packet_set_coef(&packet_table[i], i, 1);
  }

  coded_packet_t current;
  coded_packet_init(&current, L);
  coded_packet_set_coef(&current, 0, 1);
  for (i=0; i< COEF_PER_HEADER*2-1; i++) {
    coded_packet_t p;
    coded_packet_to_add(&p, &packet_table[i], &packet_table[i+1]);
    coded_packet_to_add(&current, &current, &p);
    coded_packet_adjust_min_max_coef(&current);
  }

  coded_packet_pywrite(stdout, &current);
  fprintf(stdout,"\n");

  exit(EXIT_SUCCESS);
}

/*---------------------------------------------------------------------------*/

