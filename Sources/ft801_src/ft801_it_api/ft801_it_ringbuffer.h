#ifndef _FT801_IT_RINGBUFFER_
#define _FT801_IT_RINGBUFFER_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//  Size of ring buffer must be power of 2!
//  One word is 4B long, and command data stored by api should be 4B aligned
//  but at first three bytes will be stored an address to memory
//  4*64 says that buffer can contain 64 words ( commands )
#define FT801_RINGBUFFER_SIZE 4*64



bool ft80x_it_ring_buffer_appendBuff( const uint8_t * const data, const size_t len );
bool ft80x_it_ring_buffer_append( const uint8_t data );
bool ft80x_it_ring_buffer_get( uint8_t * const data );
bool ft80x_it_ring_buffer_isfull( void );
bool ft80x_it_ring_buffer_isempty( void );
size_t ft80x_it_ring_buffer_fullness(void) ;
size_t ft80x_it_ring_buffer_freespace(void) ;

#endif