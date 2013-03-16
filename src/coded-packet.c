/*---------------------------------------------------------------------------
 * Linear coding of packets
 *---------------------------------------------------------------------------
 * Author: Cedric Adjih
 * Copyright 2013 Inria
 * All rights reserved. Distributed only with permission.
 *---------------------------------------------------------------------------*/

#include "coded-packet.h"

/*---------------------------------------------------------------------------*/


void coded_packet_copy_from(coded_packet_t* dst, coded_packet_t* src)
{ memcpy(dst, src, sizeof(*src)); }

void coded_packet_set_coef(coded_packet_t* pkt, uint16_t coef_pos,
			   uint8_t coef_value)
{
  ASSERT( pkt->log2_nb_bit_coef <= MAX_LOG2_NB_BIT_COEF );
  uint8_t l = pkt->log2_nb_bit_coef;
  uint16_t log2_window = coded_packet_log2_window(pkt);

  if (coef_value > 0) {
    if (pkt->coef_pos_min == COEF_POS_NONE 
	|| coef_pos < pkt->coef_pos_min)
      pkt->coef_pos_min = coef_pos;
    if (pkt->coef_pos_max == COEF_POS_NONE 
	|| coef_pos > pkt->coef_pos_max)
      pkt->coef_pos_max = coef_pos;

    ASSERT( pkt->coef_pos_max - pkt->coef_pos_min < (1<<log2_window) );
  }
  uint16_t actual_coef_pos = MOD_LOG2(coef_pos,  log2_window);

  lc_vector_set(pkt->content.u8, COEF_HEADER_SIZE, l, 
		actual_coef_pos, coef_value);
}

uint8_t coded_packet_get_coef(coded_packet_t* pkt, uint16_t coef_pos)
{
  ASSERT( pkt->log2_nb_bit_coef <= MAX_LOG2_NB_BIT_COEF );

  if (coef_pos < pkt->coef_pos_min || coef_pos > pkt->coef_pos_max)
    return 0;

  uint8_t l = pkt->log2_nb_bit_coef;
  uint16_t log2_window = coded_packet_log2_window(pkt);
  uint16_t actual_coef_pos = MOD_LOG2(coef_pos,  log2_window);

  return lc_vector_get(pkt->content.u8, COEF_HEADER_SIZE, l, 
		       actual_coef_pos);
}

static bool coded_packet_adjust_min_coef(coded_packet_t* pkt)
{
  REQUIRE( pkt->coef_pos_min != COEF_POS_NONE
	   && pkt->coef_pos_max != COEF_POS_NONE );

  uint8_t l = pkt->log2_nb_bit_coef;
  uint16_t log2_window = coded_packet_log2_window(pkt);
  bool result = true;
  for (;;) {
    uint16_t i = MOD_LOG2(pkt->coef_pos_min, log2_window);
    if (lc_vector_get(pkt->content.u8, COEF_HEADER_SIZE, l, i) != 0)
      break;

    pkt->coef_pos_min ++;
    if (pkt->coef_pos_min > pkt->coef_pos_max) {
      pkt->coef_pos_min = COEF_POS_NONE;
      pkt->coef_pos_max = COEF_POS_NONE;
      result = false;
      break;
    }
  }
  return result;
}

static bool coded_packet_adjust_max_coef(coded_packet_t* pkt)
{
  REQUIRE( pkt->coef_pos_min != COEF_POS_NONE
	   && pkt->coef_pos_max != COEF_POS_NONE );

  uint8_t l = pkt->log2_nb_bit_coef;
  uint16_t log2_window = coded_packet_log2_window(pkt);
  bool result = true;
  for(;;) {
    uint16_t i = MOD_LOG2(pkt->coef_pos_max, log2_window);
    if (lc_vector_get(pkt->content.u8, COEF_HEADER_SIZE, l, i) != 0)
      break;

    if (pkt->coef_pos_max == 0
	|| (pkt->coef_pos_min == pkt->coef_pos_max)) {
      pkt->coef_pos_min = COEF_POS_NONE;
      pkt->coef_pos_max = COEF_POS_NONE;
      result = false;
      break;
    }

    pkt->coef_pos_max --;
  }
  return result;
}

bool coded_packet_adjust_min_max_coef(coded_packet_t* pkt)
{
  bool result = true;
  if (pkt->coef_pos_min == COEF_POS_NONE) {
    ASSERT( pkt->coef_pos_max == COEF_POS_NONE );
    result = false;
  }
  
  if (result)
    result = coded_packet_adjust_min_coef(pkt);
  if (result)
    result = coded_packet_adjust_max_coef(pkt);
  return result;
}

bool coded_packet_is_similar(coded_packet_t* p1, coded_packet_t* p2)
{
  bool non_empty1 = coded_packet_adjust_min_max_coef(p1);
  bool non_empty2 = coded_packet_adjust_min_max_coef(p2);
  if (!non_empty1 || !non_empty2)
    return non_empty1 == non_empty2;
  if (p1->coef_pos_min != p2->coef_pos_min)
    return false;
  if (p1->coef_pos_max != p2->coef_pos_max)
    return false;  
  uint16_t i;
  for (i=p1->coef_pos_min;i<p1->coef_pos_max;i++) 
    if (coded_packet_get_coef(p1, i) != coded_packet_get_coef(p2, i))
      return false;
  uint16_t common_size = MIN(p1->data_size, p2->data_size);
  uint16_t max_size = MAX(p1->data_size, p2->data_size);
  uint8_t* d1 = coded_packet_data(p1);
  uint8_t* d2 = coded_packet_data(p2);
  for (i=0; i<common_size; i++)
    if (d1[i] != d2[i])
      return false;
  for (i=common_size; i<max_size; i++) {
    if (i < p1->data_size && d1[i] != 0)
      return false;
    if (i < p2->data_size && d2[i] != 0)
      return false;  
  }
  return true;
}

void coded_packet_init(coded_packet_t* pkt, uint8_t log2_nb_bit_coef)
{
  ASSERT( log2_nb_bit_coef <= MAX_LOG2_NB_BIT_COEF );
  pkt->log2_nb_bit_coef = log2_nb_bit_coef;
  pkt->coef_pos_min = COEF_POS_NONE;
  pkt->coef_pos_max = COEF_POS_NONE;
  pkt->data_size = 0;
  memset(pkt->content.u8, 0, COEF_HEADER_SIZE);
}

void coded_packet_init_from_base_packet
(coded_packet_t* pkt, uint8_t log2_nb_bit_coef, uint16_t base_pos,
 uint8_t* data, uint8_t data_size)
{
  coded_packet_init(pkt, log2_nb_bit_coef);  
  coded_packet_set_coef(pkt, base_pos, 1);
  ASSERT( pkt->data_size < data_size );
  memcpy(pkt->content.u8 + COEF_HEADER_SIZE, data, data_size);
  pkt->data_size = data_size;
}

/* 
   the function also operates correctly when two of the pointers (or all three)
   result, p1, or p2 are exactly the same.
*/
void coded_packet_to_add(coded_packet_t* result,
			 coded_packet_t* p1,
			 coded_packet_t* p2)
{
  ASSERT( p1->log2_nb_bit_coef == p2->log2_nb_bit_coef );

  result->log2_nb_bit_coef = p1->log2_nb_bit_coef;
  result->coef_pos_min = min_except(p1->coef_pos_min, p2->coef_pos_min,
				      COEF_POS_NONE);
  result->coef_pos_max = max_except(p1->coef_pos_max, p2->coef_pos_max,
				      COEF_POS_NONE);

  if (result->coef_pos_min == COEF_POS_NONE) {
    ASSERT( result->coef_pos_max );
    return;
  }

  ASSERT( result->coef_pos_max - result->coef_pos_min 
	  < (1<<coded_packet_log2_window(result)) );

  lc_vector_add(p1->content.u8, COEF_HEADER_SIZE + p1->data_size, 
		p2->content.u8, COEF_HEADER_SIZE + p2->data_size,
		result->content.u8, &result->data_size);
  result->data_size -= COEF_HEADER_SIZE;
}

#if 0
/* p1 pointer call be same exactly as p2 pointer */
void coded_packet_get_sum_index_bound
(coded_packet_t* p1, coded_packet_t* p2,
 uint16_t* result_coef_pos_min, uint16_t* result_coef_pos_max)
{
  *result_coef_pos_min = min_except(p1->coef_pos_min, p2->coef_pos_min,
				      COEF_POS_NONE);
  *result_coef_pos_max = max_except(p1->coef_pos_max, p2->coef_pos_max,
				      COEF_POS_NONE);
}
#endif

/* p1 pointer MUST be different from p2 pointer */
static void coded_packet_destructive_linear_combination
(uint8_t coef1, coded_packet_t* p1_and_result, 
 uint8_t coef2, coded_packet_t* p2)
{
  REQUIRE( p1_and_result != p2 );
  //if (coef1 != 1) // [XXX] uncomment for opt.
  coded_packet_to_mul(p1_and_result, coef1);
  //if (coef2 != 1) // [XXX] uncomment for opt.
  coded_packet_to_mul(p2, coef2);
  coded_packet_to_add(p1_and_result, p1_and_result, p2);
}

/* p1 += coef2 x p2 ; p1 pointer may be equal to p2 pointer */
void coded_packet_add_mult
(coded_packet_t* p1, uint8_t coef2, coded_packet_t* p2)
{
  coded_packet_t p2_copy;
  coded_packet_copy_from(&p2_copy, p2);
  coded_packet_destructive_linear_combination(1, p1, coef2, &p2_copy);
}


bool coded_packet_is_empty_safe(coded_packet_t* pkt)
{
  if (pkt->coef_pos_min == COEF_POS_NONE)
    return true;
  ASSERT(pkt->coef_pos_max != COEF_POS_NONE);
  uint16_t i;
  for (i=pkt->coef_pos_min; i<=pkt->coef_pos_max; i++)
    if (coded_packet_get_coef(pkt, i) != 0)
      return false;
  return true;
}

/*---------------------------------------------------------------------------*/

#ifdef WITH_FPRINTF
void coded_packet_pywrite(FILE* out, coded_packet_t* p)
{ 
  fprintf(out, "{ 'type':'coded-packet'");
  fprintf(out, ", 'l':%u", p->log2_nb_bit_coef);
  fprintf(out, ", 'dataSize': %u", p->data_size);
  fprintf(out, ", 'coefPosMin':%u, 'coefPosMax':%u", 
	  p->coef_pos_min, p->coef_pos_max);
  fprintf(out, ", 'coefValue':[");
  uint16_t i;
  for (i=p->coef_pos_min; i<=p->coef_pos_max; i++) {
    if (i > p->coef_pos_min)
      fprintf(out, ", ");
    fprintf(out, "%u", coded_packet_get_coef(p,i));
  }
  fprintf(out, "]");
  fprintf(out, ", data:");
  data_string_pywrite(out, coded_packet_data(p), p->data_size);
  fprintf(out, " }");
}

void coded_packet_internal_pywrite(FILE* out, coded_packet_t* p)
{ 
  fprintf(out, "{ 'type':'coded-packet'");
  fprintf(out, ", 'l':%u", p->log2_nb_bit_coef);
  fprintf(out, ", 'dataSize': %u", p->data_size);
  fprintf(out, ", 'coefPosMin':%u, 'coefPosMax':%u", 
	  p->coef_pos_min, p->coef_pos_max);
  fprintf(out, ", 'coefData':");
  data_string_pywrite(out, p->content.u8, COEF_HEADER_SIZE);
  fprintf(out, ", 'data':");
  data_string_pywrite(out, coded_packet_data(p), CODED_PACKET_SIZE);
  fprintf(out," }");  
}
#endif /* WITH_FPRINTF */

/*---------------------------------------------------------------------------*/
