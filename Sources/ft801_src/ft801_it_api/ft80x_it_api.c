#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>

#include "ft80x_it_api.h"
#include "ft80x_it_ringbuffer.h"


    
struct ft80x_it_api_data_typeDef
{
    volatile sig_atomic_t isSending ;

    // pointer to function which will enable / disable the SPI
    pf_en_TypeDef mfp_enable_spi ;
    
    // pointer to function which will enable / disable the interrupt
    pf_en_TypeDef mpf_enable_spi_it ;
    
    // pointer to function which performs reading from given address two bytes
    spi_rd_16_TypeDef mfp_spi_rd_16bit ;
    
};
    
#define IT_API_SENDING 1
#define IT_API_NOTSENDING 0


volatile static struct ft80x_it_api_data_typeDef _thisData ;








// ######### PUBLIC API #########

// Interrupt rountine
// Must be called only inside of interrupt rountine
bool ft80x_it_rountine(uint8_t * const dst )
{
    // If buffer is empty then disable sending
    if( true == ft80x_it_ring_buffer_isempty() )
    {
        // clear flags inside Interrupt-driven API
        _thisData.isSending = IT_API_NOTSENDING;
        
        // Disable an SPI
        if ( NULL != _thisData.mfp_enable_spi )
            _thisData.mfp_enable_spi(false);
        
        return false ;
    }

    
    // get data from the ringBuffer and move it to the destination
    ft80x_it_ring_buffer_get(dst) ;
    
    return true ;
}





void ft80x_it_api_init( 
    pf_en_TypeDef       fptr_spi_en,
    pf_en_TypeDef       fptr_spi_it_en,
    spi_rd_16_TypeDef   fptr_spi_rd16
)
{
    _thisData.mfp_enable_spi = fptr_spi_en ;
    _thisData.mpf_enable_spi_it = fptr_spi_it_en ;
    _thisData.mfp_spi_rd_16bit = fptr_spi_rd16 ;
    
    // Disable NVIC - for this interrupt
    _thisData.mpf_enable_spi_it(false);
    
    // Disable the spi
    _thisData.mfp_enable_spi(false);
}



// ######### SHARED API #########
// ######### User have to delare this functions inside own sources #########

void ft80x_it_start_tx()
{   
    if ( IT_API_NOTSENDING == _thisData.isSending )
    {
        _thisData.isSending = IT_API_SENDING ;
        
        _thisData.mpf_enable_spi_it(true);
        _thisData.mfp_enable_spi(true);
    }
}


void ft80x_it_spi_enable_force( bool enable )
{
    _thisData.mpf_enable_spi_it(enable) ;
    _thisData.mfp_enable_spi(enable) ;
    
    if( true == enable )
        _thisData.isSending = IT_API_SENDING ;
    else
        _thisData.isSending = IT_API_NOTSENDING ;
}


uint16_t ft80x_spi_read_16bits( const uint32_t addr )
{
    return _thisData.mfp_spi_rd_16bit( addr );
}