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

#include "dragoncast.h"

extern int32_t rand_state;

/*--------------------------------------------------*/


static void nc_bcast_event_packet(struct abc_conn *c)
{
  uint8_t* packet_data = packetbuf_dataptr();
  dg_process_coded_packet(packet_data, packetbuf_datalen());
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
void nc_bcast_send_coded_packet()
{
  packetbuf_clear();
  uint8_t* packet_data = packetbuf_dataptr();
  uint16_t packet_size = dg_generate_coded_packet(packet_data, PACKETBUF_SIZE);
  if (packet_size > 0) {
    packetbuf_set_datalen(packet_size);
    //printf("abc_send %d\n", buffer.position);
    //packetbuf_copyfrom(coded_packet, sizeof(coded_packet));
    abc_send(&abc_nc_bcast);
    //NETSTACK_MAC.send(snd_mac_callback, NULL);
  }
}

/*---------------------------------------------------------------------------*/

typedef uint32_t snd_time_t;

#define ADDRESS_SIZE 2
typedef uint8_t address_t[ADDRESS_SIZE];

#define PRINTF_ADDR(address) PRINTF("%u", ((address)[0]+((address)[1]<<8)))
#define PRINTF_ADDR_REAL(address) PRINTF_REAL("%u", ((address)[0]+((address)[1]<<8)))


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
    delay = dg_get_next_wakeup_time();
    etimer_set(&send_timer, delay);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

    nc_bcast_send_coded_packet();
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

  dg_init(node_id == 1);
  abc_open(&abc_nc_bcast, 0x42, &abc_nc_bcast_callback);
  nc_bcast_start_thread();

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

#include "dragoncast.c"

/*---------------------------------------------------------------------------*/
