/*
  DRAGONCAST:
  C. Adjih - 2013
 */



//#define COEF_PER_HEADER ((COEF_HEADER_SIZE*BITS_PER_BYTE)>>L)


#include "buffer.h"

packet_set_t packet_set;

void notify_packet_decoded(packet_set_t* packet_set, uint16_t packet_index)
{
  printf("*** decoded pkt#%u ***", packet_index);
  coded_packet_t* coded_packet= &(packet_set->coded_packet[packet_index]);
  //printf("%s\n", coded_packet->content.u8 + COEF_HEADER_SIZE);
  printf("\n");
}

void notify_set_full
(packet_set_t* packet_set, uint16_t required_min_coef_pos)
{

#if 0
  for(;;) {
    printf("packet-set full %u %u\n", packet_set_count(packet_set), required_min_coef_pos);
    printf("min %u  ", packet_set->coef_pos_min);
    //printf("pos %u\n", );
    uint16_t pkt_id = packet_set_get_id_of_pos(packet_set, packet_set->coef_pos_min);
    if (pkt_id != PACKET_ID_NONE) {
      packet_set_free_packet_id(packet_set, pkt_id);
    }
  }
#endif
  uint8_t ok = packet_set_free_first(packet_set);
  while (!ok)
    printf ("FULL: %u\n", ok);
}


static bool_t is_nc_source = 0;

void dg_init(bool_t is_source) 
{ 
  is_nc_source = is_source;
  packet_set_init(&packet_set, L, notify_packet_decoded,
		  notify_set_full, NULL);
}

/*--------------------------------------------------*/

static void generate_source_packet(uint16_t packet_id,
				   coded_packet_t* coded_packet)
{
  uint8_t packet[CODED_PACKET_SIZE];
  memset(packet, 0, CODED_PACKET_SIZE);
  snprintf((char*)packet, CODED_PACKET_SIZE, "[%u] - n%u", packet_id, node_id);
  coded_packet_init_from_base_packet
    (coded_packet, L, packet_id, packet, CODED_PACKET_SIZE);
  printf("%s\n", coded_packet->content.u8 + COEF_HEADER_SIZE);
}

static void generate_coded_source_packet(uint16_t packet_id,
					 uint16_t nb_packet,
					 coded_packet_t* coded_packet)
{
  coded_packet_t tmp_coded_packet;
  generate_source_packet(packet_id, coded_packet);
  uint16_t i;
  for (i=1; i<nb_packet; i++) {
    generate_source_packet(packet_id+i, &tmp_coded_packet);
    uint8_t coef = ((random_rand() ^ TAR) & ((1<<(1<<L))-1));
    coded_packet_add_mult( coded_packet, coef, &tmp_coded_packet);
  }
}

void dg_process_coded_packet(uint8_t* packet_data, uint16_t packet_size)
{
  printf("received packet.\n");
  coded_packet_t coded_packet;
  buffer_t buffer;
  buffer_init(&buffer, packet_data, packet_size);

  uint16_t sender_node_id = buffer_get_u16(&buffer);
  uint16_t sender_seq_num = buffer_get_u16(&buffer);
  coded_packet.log2_nb_bit_coef = L;
  coded_packet.coef_pos_min = buffer_get_u16(&buffer);
  coded_packet.coef_pos_max = buffer_get_u16(&buffer);
  uint16_t content_size = buffer_get_u16(&buffer);

  printf("packet from %u (#%u) size=%u\n", sender_node_id, sender_seq_num,
	 content_size);
  if (content_size != sizeof(coded_packet.content.u8)) {
    printf("warning: incorrect content size = %u\n", content_size);
    return;
  }
  buffer_get_data(&buffer, coded_packet.content.u8, content_size);

  coded_packet_pywrite(0, &coded_packet);
  printf("\n");
  uint16_t cycleBefore = TAR;
  static reduction_stat_t stat;
  uint16_t pkid = packet_set_add(&packet_set, &coded_packet, &stat);
  uint16_t cycleAfter = TAR;
  printf("decoding time: %u/(32K)\n", cycleAfter-cycleBefore);
  //reduction_stat_pywrite(0, &stat); printf("\n");
  //printf("%u\n", pkid);
}

/*--------------------------------------------------*/

static int current_src_pkt_id = 0;
#define PROB 0x9 /* / 0x10 */

uint16_t dg_generate_coded_packet
(uint8_t* packet_data, uint16_t max_packet_size
 /*,coded_packet_t* coded_packet */ )
{
  if (!is_nc_source)
    return 0; /* XXX: no generation */

  coded_packet_t coded_packet;
  generate_coded_source_packet(current_src_pkt_id, 8, &coded_packet);
  if (((random_rand() ^ TAR) & 0xf) < PROB)
    current_src_pkt_id++;

  buffer_t buffer;
  buffer_init(&buffer, packet_data, max_packet_size);
  buffer_put_u16(&buffer, node_id);
  buffer_put_u16(&buffer, seq_num);
  seq_num++;
  buffer_put_u16(&buffer, coded_packet.coef_pos_min);
  buffer_put_u16(&buffer, coded_packet.coef_pos_max);
  buffer_put_u16(&buffer, sizeof(coded_packet.content.u8));
  buffer_put_data(&buffer, coded_packet.content.u8,
		  sizeof(coded_packet.content.u8));
  
  if (!buffer.has_bound_error)
    return buffer.position;
  else return 0;
}

/*--------------------------------------------------*/

int long dg_get_next_wakeup_time()
{
#ifdef CONTIKI
  int long base_delay = 1 * CLOCK_SECOND /4;
  int long delay = base_delay * 3 / 4;
  uint8_t jitter = (random_rand() & 0x3);
  if (jitter == 1 || jitter == 2) delay += base_delay/4;
  else if(jitter == 3) delay += base_delay/2;
  return delay;
#else
#error NOT IMPLEMENTED YET
#warn NOT IMPLEMENTED YET
  return 1;
#endif /* CONTIKI */
}

/*--------------------------------------------------*/



/*--------------------------------------------------*/

