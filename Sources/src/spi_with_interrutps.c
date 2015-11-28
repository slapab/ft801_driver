#include "stm32f4xx.h"
#include "spi_with_interrupts.h"



#include "spi.h" // todo remove this dependacy 


void ft801_spi_hcmd_wr_it( const uint32_t cmd, const rbd_t spi_wr_descr )
{    
    // convert to 2 bytes, but skip MSB byte
    uint8_t tmp ;
    for ( int i = 2 ; i >= 0 ; --i )
    {
        tmp = (uint8_t)( cmd >> (8*i) ) ;
        ring_buffer_put( spi_wr_descr, &tmp );
    }
    
    
    // enable spi -> the interrupt will be fired
    ft801_spi_enable(true);
    
}

