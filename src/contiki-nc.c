/*---------------------------------------------------------------------------
 *  . Cedric Adjih, Hipercom Project-Team, INRIA Paris-Rocquencourt
 *  Copyright 2013 Inria.  All rights reserved.  Distributed only with permission.
 *---------------------------------------------------------------------------*/

#include <stdio.h>

#include "contiki.h"

#ifdef WITH_SHELL
#include "shell.h"
#include "serial-shell.h"
#endif /* WITH_SHELL */

#include "clock.h"

#include "net/rime.h"
#include "random.h"

#include "node-id.h"
#include "multihop.h"

#ifdef CONTIKI_TARGET_Z1
#include "dev/adxl345.h"
#endif /* CONTIKI_TARGET_Z1 */

#ifdef CONTIKI_TARGET_SKY
#include "dev/button-sensor.h"
#include "dev/light-sensor.h"
#endif /* CONTIKI_TARGET_SKY */

#ifdef CONTIKI_TARGET_Z1
#include "dev/uart0.h"
#define DBG_NOT_Z1(x) do { } while(0)
#else /* CONTIKI_TARGET_Z1 */
#include "dev/uart1.h"
#define DBG_NOT_Z1(x) do { x; } while(0)
#endif /* CONTIKI_TARGET_Z1 */

//#define WITH_OPERA_CRC
#ifdef WITH_OPERA_CRC
#include "crc16.h"
#define OPERA_CRC_XOR 0xface
#endif  /* WITH_OPERA_CRC */

#include "dev/leds.h"
#include "watchdog.h"


#include "net/netstack.h"

/*---------------------------------------------------------------------------*/

typedef uint8_t bool_t;

#define L 1
//#define COEF_PER_HEADER ((COEF_HEADER_SIZE*BITS_PER_BYTE)>>L)

#include "platform-embedded.h"
#include "coded-packet.h"
#include "packet-set.h"

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

extern int32_t rand_state;

void nc_bcast_init() 
{ 
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

/*--------------------------------------------------*/

static int current_src_pkt_id = 0;
#define PROB 0x9 /* / 0x10 */

static void nc_bcast_event_packet(struct abc_conn *c)
{
  //printf("received packet.\n");
  coded_packet_t coded_packet;
  buffer_t buffer;
  uint8_t* packet_data = packetbuf_dataptr();
  buffer_init(&buffer, packet_data, packetbuf_datalen());

  uint16_t sender_node_id = buffer_get_u16(&buffer);
  uint16_t sender_seq_num = buffer_get_u16(&buffer);
  coded_packet.log2_nb_bit_coef = L;
  coded_packet.coef_pos_min = buffer_get_u16(&buffer);
  coded_packet.coef_pos_max = buffer_get_u16(&buffer);
  uint16_t content_size = buffer_get_u16(&buffer);

  //printf("packet from %u (#%u) size=%u\n", sender_node_id, sender_seq_num,
  //content_size);
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

static struct abc_conn abc_nc_bcast;
const static struct abc_callbacks abc_nc_bcast_callback 
= { nc_bcast_event_packet };

void do_nothing()
{ }

static void snd_mac_callback(void *ptr, int status, int num_tx)
{
  printf("[snd] callback mac sent: status=%d nb-tx=%d\n", status, num_tx);
  /* XXX: maybe some stats */
}


static uint16_t seq_num = 0;
void nc_bcast_send_coded_packet(coded_packet_t* coded_packet)
{
  buffer_t buffer;
  packetbuf_clear();
  uint8_t* packet_data = packetbuf_dataptr();
  buffer_init(&buffer, packet_data, PACKETBUF_SIZE);
  buffer_put_u16(&buffer, node_id);
  buffer_put_u16(&buffer, seq_num);
  seq_num++;
  buffer_put_u16(&buffer, coded_packet->coef_pos_min);
  buffer_put_u16(&buffer, coded_packet->coef_pos_max);
  buffer_put_u16(&buffer, sizeof(coded_packet->content.u8));
  buffer_put_data(&buffer, coded_packet->content.u8,
		  sizeof(coded_packet->content.u8));
  
  packetbuf_set_datalen(buffer.position);
  //printf("abc_send %d\n", buffer.position);
  //packetbuf_copyfrom(coded_packet, sizeof(coded_packet));
  abc_send(&abc_nc_bcast);
  //NETSTACK_MAC.send(snd_mac_callback, NULL);
}

void nc_bcast_generate_src()
{
  coded_packet_t coded_packet;
  generate_coded_source_packet(current_src_pkt_id, 8, &coded_packet);
  //printf("generate:");
  //coded_packet_pywrite(0, &coded_packet);
  //printf("\n");
  if (((random_rand() ^ TAR) & 0xf) < PROB)
    current_src_pkt_id++;

  nc_bcast_send_coded_packet(&coded_packet);
}

/*---------------------------------------------------------------------------*/

typedef uint32_t snd_time_t;

#define ADDRESS_SIZE 2
typedef uint8_t address_t[ADDRESS_SIZE];

#define PRINTF_ADDR(address) PRINTF("%u", ((address)[0]+((address)[1]<<8)))
#define PRINTF_ADDR_REAL(address) PRINTF_REAL("%u", ((address)[0]+((address)[1]<<8)))

void nc_bcast_generate()
{
  if (node_id == 1) {
    nc_bcast_generate_src();
  }
}

PROCESS(nc_bcast_thread, "nc");
PROCESS_THREAD(nc_bcast_thread, ev, data)
{
  static struct etimer send_timer;
  static int long delay;
  static snd_time_t last_write_time; 
  
  PROCESS_BEGIN();
  random_init(node_id);

  address_t address;
  memset(address, 0, sizeof(address));
  memcpy(address, &node_id, sizeof(node_id)); /* XXX: not portable */

  for (;;) {
    int long base_delay = 1 * CLOCK_SECOND /4;
    delay = base_delay * 3 / 4;
    uint8_t jitter = (random_rand() & 0x3);
    if (jitter == 1 || jitter == 2) delay += base_delay/4;
    else if(jitter == 3) delay += base_delay/2;

    etimer_set(&send_timer, delay);

    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

    nc_bcast_generate();
  }

  PROCESS_END();
}

void nc_bcast_start_thread()
{
  process_start(&nc_bcast_thread, NULL);
}

/*---------------------------------------------------------------------------*/

PROCESS(init_process, "init");
AUTOSTART_PROCESSES(&init_process);

PROCESS_THREAD(init_process, ev, data)
{
  PROCESS_BEGIN();

  nc_bcast_init();
  abc_open(&abc_nc_bcast, 0x42, &abc_nc_bcast_callback);
  nc_bcast_start_thread();

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
