#include "stm32f4xx.h"
#include "spi.h"

#include "ft801_gpu.h"    // FT801 registers 
#include "ft801_api.h"    // include api functions

#include "ringBuffer_api.h"

#include <assert.h>

// Declare the descryptor for ring buffer
rbd_t rb_spi_rd_descr ;
rbd_t rb_spi_wr_descr ;



void SPI4_IRQHandler(void)
{
    // Receive
    if( READ_BIT(SPI4->SR, SPI_SR_RXNE) )
    {
        uint8_t tmp = SPI4->DR;
        // Put to the buffer
        ring_buffer_put(rb_spi_rd_descr, (const uint8_t*)&tmp);
    }
    
    
    // Transmit
    if ( READ_BIT( SPI4->SR, SPI_SR_TXE) )
    {
        uint8_t tmp = 0 ;
        // get from buffer and put to the SPI tx register only if were something
        if ( ring_buffer_get( rb_spi_wr_descr, &tmp) )
        {
            SPI4->DR = tmp ;
        }
    }
    
    
    // if SPI operation has been ended then disable the SPI
    // todo -> inform that transmision has been stoped
    if (!(SPI4->SR & SPI_SR_BSY))
    {
        ft801_spi_enable(false) ;
        // to do here
    }
    
    
    NVIC_ClearPendingIRQ(SPI4_IRQn);
}




void sleep( uint32_t delay )
{
    for ( uint32_t i = 0 ; i < 1600000; ++i ) ;
}


int main(void)
{
    
    // declare the memory for ring buffer
    uint8_t pSPI_buff_rd[16]; // read ring buffer
    uint8_t pSPI_buff_wr[16]; // write ring buffer
    
    
    // declare the ring buffer structure and init it
    rb_attr_t ringBuffer_prepare =
                {
                    sizeof( pSPI_buff_rd[0]),
                    sizeof( pSPI_buff_rd ) / sizeof( pSPI_buff_rd[0]),
                    (void*)&pSPI_buff_rd[0]
                } ;
                
    // add defined ring buffer to the system and get the descriptor
    if ( !ring_buffer_init(&rb_spi_rd_descr, &ringBuffer_prepare) )
    {
        assert(!"Can not init ring buffer") ;
    }
    
    // Prepare second buffer
    ringBuffer_prepare.s_elem = sizeof( pSPI_buff_wr[0] );
    ringBuffer_prepare.n_elem = sizeof( pSPI_buff_wr ) / sizeof ( pSPI_buff_wr[0] );
    ringBuffer_prepare.buffer = (void*)&pSPI_buff_wr[0] ;
    
    // add defined ring buffer to the system and get the descriptor
    if ( !ring_buffer_init(&rb_spi_wr_descr, &ringBuffer_prepare) )
    {
        assert(!"Can not init ring buffer") ;
    }
    
    
    
    gpu_init_spi() ;
    

//    NVIC_ClearPendingIRQ(SPI4_IRQn) ;
//    NVIC_EnableIRQ(SPI4_IRQn) ;
//    cnt = 0 ;
    
    // init sequence
    ft801_spi_hcmd_write( FT_HOST_CMD_CLKEXT ) ;
    ft801_spi_hcmd_write( FT_HOST_CMD_ACTIVE ) ;
    

    uint8_t ret = 0 ;
    ret = ft801_spi_rd8( REG_ID );
    
    if ( ret == 0x7C )
    {
        ft801_api_init_lcd() ;
        
        // test if write was done fine and test if read function works
        if ( ft801_spi_mem_rd16(REG_HCYCLE) == 548 ) {
            int lt = 0 ;
        }
    }        


    
	SystemCoreClockUpdate();
	uint32_t a = SystemCoreClock  ;
//	uint32_t b = 0 ;
    return 0 ;
}
