#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>

#include "ft801_gpu.h"      // all about FT801 chip

#include "ft80x_it_api.h"
#include "ft80x_it_ringbuffer.h"


    
struct ft80x_it_api_data_typeDef
{
    volatile sig_atomic_t m_sendingStatus ;
    
    volatile size_t chip_fifo_write ; //using for update the chip's FIFO write reg.
    
    
    // pointer to function which will enable / disable the SPI
    pf_en_TypeDef mfp_enable_spi ;
    
    // pointer to function which will enable / disable the interrupt
    pf_en_TypeDef mpf_enable_spi_it ;
    
    // pointer to function which performs reading from given address two bytes
    spi_rd_16_TypeDef mfp_spi_rd_16bit ;
    
};
typedef enum
{
  ReadyToSend = 0,
  SendingData,
  SendingRegData,
  SentData,
  SentRegData
} sendingStatus_TypeDef ;

//#define IT_API_SENDING 1
//#define IT_API_NOTSENDING 0


volatile static struct ft80x_it_api_data_typeDef _thisData ;








// ######### PUBLIC API #########

// Interrupt rountine
// Must be called only inside of interrupt rountine
bool ft80x_it_rountine(uint8_t * const dst )
{
    // If buffer is empty then disable sending
    if( false == ft80x_it_ring_buffer_get(dst) )
    {
        
        // inform it_engine about end of transmission
        if ( SendingData == _thisData.m_sendingStatus )
            _thisData.m_sendingStatus = SentData ;
        else if ( SendingRegData == _thisData.m_sendingStatus  )
            _thisData.m_sendingStatus = SentRegData ;
        
        
        // Disable an SPI
//        if ( NULL != _thisData.mfp_enable_spi )
//        {
        _thisData.mpf_enable_spi_it(false) ;
            _thisData.mfp_enable_spi(false);
        
        //}
        
        
        return false ;
    }

    
    return true ;
}




// Must be called in the loop - to ends the transmissions
bool ft80x_it_check(void)
{
    sig_atomic_t status = _thisData.m_sendingStatus ;
    if ( SentData == status )
    {
        // Tell the chip to start processing commands
        
        // Append dst address to the buffer -> MSB first
        uint32_t addr = REG_CMD_WRITE ;
        addr |= 0x00800000 ;    // Set bit that this will be write transaction
        ft80x_it_ring_buffer_append( (uint8_t)(addr >> 16) );
        ft80x_it_ring_buffer_append( (uint8_t)(addr >> 8) );
        ft80x_it_ring_buffer_append( addr );
        
        // push the value -> LSB first -> Little Endian
        size_t tmp = _thisData.chip_fifo_write ;
        ft80x_it_ring_buffer_append( (uint8_t)tmp ) ;
        ft80x_it_ring_buffer_append( (uint8_t)(tmp >> 8) ) ;
        ft80x_it_ring_buffer_append( (uint8_t)(tmp >> 16) ) ;
        ft80x_it_ring_buffer_append( (uint8_t)(tmp >> 24) ) ;
        
        // set status
        _thisData.m_sendingStatus = SendingRegData ;
        
        // enable spi and insterrupt -> start sending
        _thisData.mpf_enable_spi_it(true) ;
        _thisData.mfp_enable_spi(true) ;
        
        
        return false ;
    }
    else if ( SentRegData == status )
    {
        // Whole cmds were sent
        _thisData.m_sendingStatus = ReadyToSend ;
        
        return true ;
    }
    else
    {
        // Unknown state
        return false ;
    }
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

void ft80x_it_start_tx_cmds( size_t chip_fifo_wr )
{   
   
    // save this data in the structure
    _thisData.chip_fifo_write = chip_fifo_wr ;
    
    
    // start IT sending
    if ( ReadyToSend == _thisData.m_sendingStatus )
    {
        _thisData.m_sendingStatus = SendingData ;
        
        _thisData.mpf_enable_spi_it(true);
        _thisData.mfp_enable_spi(true);
    }
}



/** It resets state of interrupt driver
*   But it resets only if there is no transmission in progress
*   @return false if transmission is in progress and can not reset the state,
*           true otherwise.
*   
*/
bool ft80x_it_reset( bool force )
{
    // check state before before reseting state
    if ( false == force )
    {
        if ( ReadyToSend != _thisData.m_sendingStatus )
            return false ;
    }
    
    _thisData.mpf_enable_spi_it(false) ;
    _thisData.mfp_enable_spi(false) ;

    _thisData.m_sendingStatus = ReadyToSend ;
    
    return true ;
}


uint16_t ft80x_spi_read_16bits( const uint32_t addr )
{
    return _thisData.mfp_spi_rd_16bit( addr );
}






// ####### LOCAL - HELPER - FUNCTIONS #######


