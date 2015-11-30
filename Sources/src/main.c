#include "stm32f4xx.h"
#include "spi.h"

#include "spi_with_interrupts.h"


#include "ft801_gpu.h"    // FT801 registers
#include "ft801_api.h"    // include api functions

#include "ringBuffer_api.h"

#include <assert.h>

// Declare the descryptor for ring buffer
rbd_t rb_spi_rd_descr ;
rbd_t rb_spi_wr_descr ;
// declare the memory for ring buffer
volatile uint8_t pSPI_buff_rd[16]; // read ring buffer
volatile uint8_t pSPI_buff_wr[16]; // write ring buffer

volatile size_t cnt = 0 ;

void SPI4_IRQHandler(void)
{
    // Receive
    if( READ_BIT(SPI4->SR, SPI_SR_RXNE) )
    {
        uint8_t tmp = SPI4->DR;
        // Put to the buffer
//        cnt++ ;
//        if ( cnt > 3 )
            ring_buffer_put(rb_spi_rd_descr, (const uint8_t*)&tmp);
    }
    
    
    // Transmit
    if ( READ_BIT( SPI4->SR, SPI_SR_TXE) )
    {
        uint8_t tmp = 0 ;
        // get from buffer and put to the SPI tx register only if were something
        if (ring_buffer_get( rb_spi_wr_descr, &tmp) )
        {
            if ( (SPI4->CR1 & SPI_CR1_SPE) )
                SPI4->DR = tmp ;
        }
    }
    
    
    // if SPI operation has been ended then disable the SPI
    // todo -> inform that transmision has been stoped
    if ( ring_buffer_get_capacity(rb_spi_wr_descr) == 0 )//!(SPI4->SR & SPI_SR_BSY))
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
    
    
    // init the SPI
    gpu_init_spi() ;
    
    
//    NVIC_EnableIRQ(SPI4_IRQn) ;
//    ft801_spi_hcmd_wr_it(FT_HOST_CMD_CLKEXT, rb_spi_wr_descr) ;
    
    
    
//    NVIC_ClearPendingIRQ(SPI4_IRQn) ;
    
//    cnt = 0 ;
    
    // init sequence
    ft801_spi_host_cmd( FT_HOST_CMD_CLKEXT ) ;
    ft801_spi_host_cmd( FT_HOST_CMD_48M ) ;
    ft801_spi_host_cmd( FT_HOST_CMD_ACTIVE ) ;
    
    ft801_spi_host_cmd( FT_HOST_CMD_CORERST ) ;
    
    sleep(2) ;
    
    
    uint8_t ret = 0 ;
    ret = ft801_spi_rd8( REG_ID );
    
    
    
    if ( ret == 0x7C )
    {
        ft801_api_init_lcd() ;

        // set the backlight pwm duty 0-128
         ft801_spi_mem_wr8(REG_PWM_DUTY, 5) ;
        
        // draw the single point
//          ft801_spi_enable(true);
    
//        ft801_spi_mem_wr32(RAM_DL, CLEAR_COLOR_RGB(0,255,0), true);
//        ft801_spi_mem_wr32(RAM_DL+4, CLEAR(1, 0, 0), true);
//        ft801_spi_mem_wr32(RAM_DL+8, DISPLAY(), true);
        
        ft801_spi_mem_wr32(RAM_DL, CLEAR(1, 1, 1));
        ft801_spi_mem_wr32(RAM_DL+4, POINT_SIZE(320));
        
        ft801_spi_mem_wr32(RAM_DL+8, BEGIN(POINTS));
        ft801_spi_mem_wr32(RAM_DL+12, COLOR_RGB(160, 22, 22));
        ft801_spi_mem_wr32(RAM_DL+16, VERTEX2II(110, 20, 0, 0));
        
        ft801_spi_mem_wr32(RAM_DL+20, COLOR_RGB(160, 255, 255));
        ft801_spi_mem_wr32(RAM_DL+24, VERTEX2II(240, 136, 0, 0));
        
        ft801_spi_mem_wr32(RAM_DL+28, END());
        
        ft801_spi_mem_wr32(RAM_DL+32, DISPLAY());
        
        
        ft801_spi_mem_wr8(REG_DLSWAP, DLSWAP_FRAME); // dispaly
//        ft801_spi_enable(false);
        
        if ( false == ft801_api_is_enabled() )
        {
            ft801_api_enable_lcd(true);
        }
        
        //ft801_spi_mem_wr8( REG_PCLK, 10, true ) ; // enable pll - whole gpu
        
//        // test if write was done fine and test if read function works
//        if ( (ft801_spi_rd16(REG_HCYCLE) == 548) &&
//             (ft801_spi_rd16(REG_HOFFSET) == 43 ) )
//        {
//            volatile int lt = 0 ;
//            ++lt ;
//            
////            ft801_spi_mem_wr8(REG_PWM_DUTY, 6, true) ;
////            if ( ft801_spi_rd8(REG_PWM_DUTY) == 6 )
////            {
////                ++lt ;
////            }
//        }
    }        


    
	SystemCoreClockUpdate();
	uint32_t a = SystemCoreClock  ;
//	uint32_t b = 0 ;
    return 0 ;
}
