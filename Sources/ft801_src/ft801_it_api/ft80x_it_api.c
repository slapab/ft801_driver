#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>

#include "ft80x_it_api.h"
#include "ft80x_it_ringbuffer.h"



static sig_atomic_t ft80x_it_api_flags ;
#define IT_API_FLAG_NOTSENDING 0
#define IT_API_FLAG_SENDING 1






// Interrupt rountine
// Must be called only inside of interrupt rountine
bool ft80x_it_rountine(uint8_t * const dst )
{
    // If buffer is empty then disable sending
    if( true == ft80x_it_ring_buffer_isempty() )
    {
        // clear flags inside Interrupt-driven API
        ft80x_it_api_flags = IT_API_FLAG_NOTSENDING; 
        
        return false ;
    }

    // if buffer is not empty then send the data 
    
    
    // get data from the ringBuffer and move it to the destination
    ft80x_it_ring_buffer_get(dst) ;
    
    return true ;
}




