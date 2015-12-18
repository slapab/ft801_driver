#include "ft80x_it_ringbuffer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>


// the ring buffer structure
struct ft80x_it_ring_buffer
{
    volatile uint8_t  buf[ FT801_RINGBUFFER_SIZE ];
    volatile sig_atomic_t head;
    volatile sig_atomic_t tail;
};


// private instance of ring buffer
static volatile struct ft80x_it_ring_buffer ringBuffer;




// api to handle the ring buffer
bool ft80x_it_ring_buffer_appendBuff( const char * const data, const size_t len )
{
    
    if ( !ft80x_it_ring_buffer_isfull() )
    {
        size_t free_space = ft80x_it_ring_buffer_freespace() ;
        if ( free_space < len )
        {
            return false ;
        }
        
        // instead of modulo operation -> but the size must be power of 2
        sig_atomic_t head_idx = ringBuffer.head & (FT801_RINGBUFFER_SIZE-1); 
        
        for ( size_t i = 0 ; i < len ; ++i )
        {
            ringBuffer.buf[ head_idx ] = data[i] ;
            head_idx = (head_idx +1) & (FT801_RINGBUFFER_SIZE-1);
        }
       
        // update index
        ringBuffer.head += len;
        return true ;
    }
    else
    {
        return false;
    }
    
}


bool ft80x_it_ring_buffer_append( const uint8_t data )
{
    if ( ft80x_it_ring_buffer_isfull() )
        return false ;
    
    
    // instead of modulo operation -> but the size must be power of 2
    sig_atomic_t head_idx = ringBuffer.head & (FT801_RINGBUFFER_SIZE-1); 
   
    // append to the buffer
    ringBuffer.buf[ head_idx ] = data ;
   
    // update index
    ++ringBuffer.head;
    return true ;
}


// full if head+1 == tail
// empty if tail == head
bool ft80x_it_ring_buffer_isfull( void )
{
    // add 1 to handle this implementation of ringBuffer
    sig_atomic_t head_idx = (ringBuffer.head+1) & (FT801_RINGBUFFER_SIZE-1) ;
    sig_atomic_t tail_idx = ringBuffer.tail & (FT801_RINGBUFFER_SIZE-1) ;
    
    return ( head_idx == tail_idx ) ? true : false ;
}

bool ft80x_it_ring_buffer_isempty( void )
{ 
    return ( ringBuffer.head == ringBuffer.tail )? true : false ;
}


size_t ft80x_it_ring_buffer_fullness(void)
{
    size_t fullness = (ringBuffer.head - ringBuffer.tail) & (FT801_RINGBUFFER_SIZE-1);
    return fullness ;
}


size_t ft80x_it_ring_buffer_freespace(void)
{   
    size_t fullness = ft80x_it_ring_buffer_fullness() ;
    return FT801_RINGBUFFER_SIZE - fullness - 1 ; // -1 to handle this implementation of ring buffer
}




// get form ring buffer
bool ft80x_it_ring_buffer_get( uint8_t * const data )
{
    if ( ft80x_it_ring_buffer_isempty() )
        return false ;
    
    sig_atomic_t tail_idx = ringBuffer.tail & ( FT801_RINGBUFFER_SIZE -1 );
    uint8_t tmp = ringBuffer.buf[tail_idx] ;
    *data = tmp ;
    
    ++ringBuffer.tail ;
    
    return true ;
}



void ft80x_it_ring_buffer_reset(void)
{
    ringBuffer.head = ringBuffer.tail = 0 ;
}