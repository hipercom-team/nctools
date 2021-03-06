/** \addtogroup core 
    @{ */

/**
 * \defgroup buffer 
 * @{
 *
 * \file 
 *   Buffer related functions:
 *     A buffer is a sequence of bytes, in which one can either push or pop: 
 *     block of bytes, unsigned 8-bit integers, unsigned 16-bit
 *     integers or unsigned 32-bit integers, in network byte order.
 *
 *     These are used for packet generation and parsing.
 *     
 *     Operations on unstructured data are the macros BLOCK_...
 *     Operations on 'struct' buffer are the macros BUFFER_...
 *     and equivalent inline functions are available as buffer_...
 *
 * \author Cedric Adjih <cedric.adjih@inria.fr>
 */

/* copied from contiki-senslab-unified/hipercom/node_ui/buffer.h 
   by C.A., 16 June 2013 */

#ifndef __BUFFER_H__
#define __BUFFER_H__

/*---------------------------------------------------------------------------*/

#include <string.h>

/*--------------------------------------------------*/

#include "general.h"

/*---------------------------------------------------------------------------*/

/* internal macros */

/** \brief Internal macros */
#define BLOCK_ERROR(err) BEGIN_MACRO err; END_MACRO
#define BEGIN_MACRO_CHECK_SIZE(max_data_size, pos, added_data_size, err) \
  BEGIN_MACRO  \
  if ( (pos)+(added_data_size) > (max_data_size)) { BLOCK_ERROR(err); } \
  else

/*---------------------------------------------------------------------------*/

/** \brief Put an unsigned 16 bit integer at pointer \a data */
#define DIRECT_PUT_U16(data, value)					\
  BEGIN_MACRO {								\
    (data)[0] = (((uint16_t)(value))>>8) & 0xffu;			\
    (data)[1] = ((uint16_t)(value)) & 0xffu;				\
  } END_MACRO

/** \brief Get an unsigned 16 bit integer at pointer \a data */
#define DIRECT_GET_U16(data)				    \
  ((((uint16_t)((data)[0])) << 8)			    \
   |((uint16_t)((data)[1])))

/** \brief Put an unsigned 32 bit integer at pointer \a data */
#define DIRECT_PUT_U32(data, value)			    \
  BEGIN_MACRO {						    \
    (data)[0] = (((uint32_t)(value))>>(3*8)) & 0xffu;	    \
    (data)[1] = (((uint32_t)(value))>>(2*8)) & 0xffu;	    \
    (data)[2] = (((uint32_t)(value))>>(1*8)) & 0xffu;	    \
    (data)[3] = ((uint32_t)(value)) & 0xffu;		    \
  } END_MACRO

/** \brief Get an unsigned 32 bit integer at pointer \a data */
#define DIRECT_GET_U32(data)				    \
  ( (((uint32_t)((data)[0])) << (3*8))			    \
    | (((uint32_t)((data)[1])) << (2*8))		    \
    | (((uint32_t)((data)[2])) << (1*8))		    \
    | ((uint32_t)((data)[3])))

/*---------------------------------------------------------------------------*/

/** \brief Append some data to a block */
#define BLOCK_PUT_DATA(data, max_data_size, pos,			\
		       added_data, added_data_size, err)		\
  BEGIN_MACRO_CHECK_SIZE(max_data_size, pos, added_data_size, err) { 	\
    memcpy((data)+(pos), (added_data), (added_data_size));		\
    pos += (added_data_size);						\
  } END_MACRO

/** \brief Append one unsigned byte to a block */
#define BLOCK_PUT_U8(data, max_data_size, pos, value, err)  \
  BEGIN_MACRO_CHECK_SIZE(max_data_size, pos, 1, err) {	    \
    (data)[(pos)] = (uint8_t)(value);			    \
    pos ++;						    \
  } END_MACRO;

/** \brief Append one unsigned short to a block */
#define BLOCK_PUT_U16(data, max_data_size, pos, value, err)	    \
  BEGIN_MACRO_CHECK_SIZE(max_data_size, pos, 2, err) {		    \
    DIRECT_PUT_U16( ((data)+(pos)),  value);			    \
    pos +=2 ;							    \
  } END_MACRO;

/** \brief Append one unsigned 32 bit integer to a block */
#define BLOCK_PUT_U32(data, max_data_size, pos, value, err)	    \
  BEGIN_MACRO_CHECK_SIZE(max_data_size, pos, 4, err) {		    \
    DIRECT_PUT_U32( ((data)+(pos)),  value);			    \
    pos +=4 ;							    \
  } END_MACRO;

/*--------------------------------------------------*/

/** \brief Get some data from a block */
#define BLOCK_GET_DATA(popped_data, popped_data_size,\
		       data, max_data_size, pos,   \
		       err)						\
  BEGIN_MACRO_CHECK_SIZE(max_data_size, pos, popped_data_size, err) { 	\
    memcpy((popped_data), (data)+(pos), (popped_data_size));		\
    pos += (popped_data_size);						\
  } END_MACRO

/** \brief Pop one unsigned byte from a block */
#define BLOCK_GET_U8(value, data, max_data_size, pos, err)	\
  BEGIN_MACRO_CHECK_SIZE(max_data_size, pos, 1, err) {		\
    value = (data)[(pos)];					\
    pos ++;							\
  } END_MACRO;

/** \brief Pop one unsigned 16 bit integer from a block */
#define BLOCK_GET_U16(value, data, max_data_size, pos, err)	    \
  BEGIN_MACRO_CHECK_SIZE(max_data_size, pos, 2, err) {		    \
    value = DIRECT_GET_U16( (data)+(pos) );			    \
    pos +=2 ;							    \
  } END_MACRO;

/** \brief Pop one unsigned 32 bit integer from a block */
#define BLOCK_GET_U32(value, data, max_data_size, pos, err)	    \
  BEGIN_MACRO_CHECK_SIZE(max_data_size, pos, 4, err) {		    \
    value = DIRECT_GET_U32( (data)+(pos));			    \
    pos +=4 ;							    \
  } END_MACRO;


/** \brief Get some data from a block */
#define BLOCK_PEEK_DATA(popped_data_size,\
			data, max_data_size, pos,			\
			err)						\
  BEGIN_MACRO_CHECK_SIZE(max_data_size, pos, popped_data_size, err) { 	\
    pos += (popped_data_size);						\
  } END_MACRO


/*---------------------------------------------------------------------------*/

/**
 * \struct buffer_t
 * \brief A buffer: it is a sequence of bytes starting from pointer `data'
 *   with (maximum) size `capacity' and where `position' acts as a cursor.
 *
 *   It must be initialized by `BUFFER_INIT' or `buffer_init'
 *   (then `position' equals 0 and `has_bound_error' is reset to BOOL_FALSE)
 *
 *   - One can push and pop data, with BUFFER_PUT_... or BUFFER_GET_... macros;
 *   with these functions, the current `position' is properly incremented,
 *   and tracks the last unused byte.
 *   - One can alternatively push and pop data, with buffer_put_... or 
 *   buffer_get_... functions (instead of macros) ; and in this case, 
 *   bounds errors are handled (moreover they are checked with unit tests).
 */

typedef struct {
  uint8_t* data;            /**< pointer to underlying block of memory */
  unsigned int capacity;    /**< maximum amount of bytes associated to `data' */
  unsigned int position;    /**< current position (for next put/get data) */ 
  bool_t   has_bound_error; /**< BOOL_TRUE iff an overflow was detected */
} buffer_t;

/*--------------------------------------------------*/

/** \brief Initialize the content of one buffer */
#define BUFFER_INIT(buffer, data, capacity)	    \
  BEGIN_MACRO { 				    \
    (buffer).data = (data);			    \
    (buffer).capacity = (capacity);		    \
    (buffer).position = 0;			    \
  } END_MACRO

/*--------------------------------------------------*/

/**
 * \name BUFFER_PUT_... macros
 * @{
 */

/** \brief Append a sequence of bytes a buffer */
#define BUFFER_PUT_DATA(buffer, added_data, added_data_size, err) \
  BLOCK_PUT_DATA((buffer).data, (buffer).capacity, (buffer).position, \
		 (added_data), (added_data_size), err)

/** \brief Append one byte a buffer */
#define BUFFER_PUT_U8(buffer, value, err) \
  BLOCK_PUT_U8((buffer).data, (buffer).capacity, (buffer).position,	\
	       (value), err)

/** \brief Append a 16-bit unsigned integer to a buffer */
#define BUFFER_PUT_U16(buffer, value, err) \
  BLOCK_PUT_U16((buffer).data, (buffer).capacity, (buffer).position,	\
	       (value), err)

/** \brief Append a 32-bit unsigned integer to a buffer */
#define BUFFER_PUT_U32(buffer, value, err) \
  BLOCK_PUT_U32((buffer).data, (buffer).capacity, (buffer).position,	\
	       (value), err)

/** @} */

/*--------------------------------------------------*/

#define BUFFER_GET_DATA(popped_data, popped_data_size, buffer, err)    \
  BLOCK_GET_DATA(popped_data, popped_data_size, (buffer).data, \
		 (buffer).capacity, (buffer).position, err)

#define BUFFER_GET_U8(value, buffer, err)				\
  BLOCK_GET_U8(value, (buffer).data, (buffer).capacity, (buffer).position, \
	       err)

#define BUFFER_GET_U16(value, buffer, err)				\
  BLOCK_GET_U16(value, (buffer).data, (buffer).capacity, \
		(buffer).position, err)

#define BUFFER_GET_U32(value, buffer, err)				\
  BLOCK_GET_U32(value, (buffer).data, (buffer).capacity,		\
		(buffer).position, err)

/*---------------------------------------------------------------------------*/

/**
 * \name buffer__... inline functions
 * @{
 */

static inline void buffer_init(buffer_t* buffer, uint8_t* data, 
			       unsigned int capacity)
{ 
  BUFFER_INIT( (*buffer), data, capacity); 
  buffer->has_bound_error = 0;
}

static inline void buffer_init_from_part(buffer_t* buffer, 
					 buffer_t* super_buffer,
					 unsigned int size)
{ 
  if (super_buffer->capacity - super_buffer->position + 1 < size) {
    super_buffer->position = super_buffer->capacity;
    super_buffer->has_bound_error = BOOL_TRUE;
    buffer->data = NULL;
    buffer->capacity = 0;
    buffer->position = 0;
    buffer->has_bound_error = BOOL_TRUE;
    return;
  }

  buffer_init(buffer, super_buffer->data + super_buffer->position, size);
  super_buffer->position += size;
}


static inline void buffer_put_data(buffer_t* buffer, 
				   uint8_t* added_data, 
				   unsigned int added_data_size)
{ BUFFER_PUT_DATA( (*buffer), added_data, added_data_size, 
		  (buffer)->has_bound_error = BOOL_TRUE); }



#define BUFFER_PEEK_DATA(popped_data_size, buffer, err)		\
  BLOCK_PEEK_DATA(popped_data_size, (buffer).data,		\
		  (buffer).capacity, (buffer).position, err)

static inline uint8_t* buffer_peek_data(buffer_t* buffer, 
					unsigned int data_size)
{ 
  if (buffer->position+data_size <= buffer->capacity) { 
    uint8_t* result = buffer->data + buffer->position;
    buffer->position += data_size;
    return result;
  } else {
    buffer->has_bound_error = BOOL_TRUE;
    return NULL;
  }
}

static inline void buffer_put_u8(buffer_t* buffer, uint8_t value)
{ BUFFER_PUT_U8( (*buffer), value, (buffer)->has_bound_error = BOOL_TRUE); }

static inline void buffer_put_u16(buffer_t* buffer, uint16_t value)
{ BUFFER_PUT_U16( (*buffer), value, (buffer)->has_bound_error = BOOL_TRUE); }

static inline void buffer_put_u32(buffer_t* buffer, uint32_t value)
{ BUFFER_PUT_U32( (*buffer), value, (buffer)->has_bound_error = BOOL_TRUE); }


static inline void buffer_get_data
(buffer_t* buffer, uint8_t* popped_data, unsigned int popped_data_size)
{ BUFFER_GET_DATA( popped_data, popped_data_size, (*buffer),
		  (buffer)->has_bound_error = BOOL_TRUE); }

static inline uint8_t buffer_get_u8(buffer_t* buffer)
{ 
  uint8_t result;
  BUFFER_GET_U8(result, (*buffer), 
		(buffer)->has_bound_error = BOOL_TRUE; result=0); 
  return result;
}

static inline uint16_t buffer_get_u16(buffer_t* buffer)
{ 
  uint16_t result;
  BUFFER_GET_U16(result, (*buffer), 
		 (buffer)->has_bound_error = BOOL_TRUE; result=0); 
  return result;
}

/** \brief */
static inline uint32_t buffer_get_u32(buffer_t* buffer)
{ 
  uint32_t result;
  BUFFER_GET_U32(result, (*buffer), 
		 (buffer)->has_bound_error = BOOL_TRUE; result=0); 
  return result;
}

/** @} */

/*---------------------------------------------------------------------------*/

typedef struct {
  uint8_t position;
} buffer_mark_t;

buffer_mark_t buffer_put_mark_u8(buffer_t* buffer)
{
  buffer_mark_t result;
  result.position = buffer->position;
  buffer_put_u8(buffer, 0);
  return result;
}

void buffer_put_size_at_mark_u8(buffer_t* buffer, buffer_mark_t mark, 
				int offset)
{
  uint8_t value = buffer->position - mark.position + offset - 1;
  if (mark.position >= buffer->capacity) {
    buffer->has_bound_error = BOOL_TRUE;
    return;
  }
  buffer->data[mark.position] = value;
}

/*---------------------------------------------------------------------------*/

#if 0

typedef struct {
  uint8_t current_byte;
  uint8_t current_bit_pos;
} bit_buffer_t;

static inline void bit_buffer_init_write(bit_buffer_t* bit_buffer) 
{
  bit_buffer->current_byte = 0;
  bit_buffer->current_bit_pos = 0;
}

static inline void bit_buffer_flush(buffer_t* buffer, bit_buffer_t* bit_buffer)
{
  if (bit_buffer->current_bit_pos > 0) {
    buffer_put_u8(buffer, bit_buffer->current_byte);
    bit_buffer->current_byte = 0;
    bit_buffer->current_bit_pos = 0;
  }
}

static inline void bit_buffer_init_read
(buffer_t* buffer, bit_buffer_t* bit_buffer)
{
  bit_buffer->current_byte = buffer_get_u8(buffer);
  bit_buffer->current_bit_pos = 0;
}

static inline void bit_buffer_put
(buffer_t* buffer, bit_buffer_t* bit_buffer, bool_t bit)
{
  if (bit)
    bit_buffer->current_byte |= 1u<<(bit_buffer->current_bit_pos);
  bit_buffer->current_bit_pos += 1;
  if (bit_buffer->current_bit_pos == BITS_PER_BYTE)
    bit_buffer_flush();
}

static inline bool_t bit_buffer_get(buffer_t* buffer, bit_buffer_t* bit_buffer)
{
  if (bit_buffer->current_bit_pos == 8)
    bit_buffer_init_read(buffer, bit_buffer);
  bool_t result = (bit_buffer->current_byte 
		   & (1u << bit_buffer->current_bit_pos)) != 0;
  bit_buffer->current_bit_pos ++;
  return result;
}

#endif

/*---------------------------------------------------------------------------*/

/** @} */ 
/** @} */ 

#endif /* __BUFFER_H__ */
