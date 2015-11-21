#include "stm32f4xx.h"
#include "spi.h"

#include "ft801_gpu.h"    // FT801 registers 
#include "ft801_api.h"    // include api functions

volatile uint8_t value[8] ;
volatile uint8_t cnt = 0 ;

void SPI4_IRQHandler(void)
{
    if( READ_BIT(SPI4->SR, SPI_SR_RXNE) )
    {
        value[cnt] = SPI4->DR ;
        ++cnt;
    }
    NVIC_ClearPendingIRQ(SPI4_IRQn);
}


void sleep( uint32_t delay )
{
    for ( uint32_t i = 0 ; i < 1600000; ++i ) ;
}

int main(void)
{
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

    while( cnt < 5 ) ;
    
	SystemCoreClockUpdate();
	uint32_t a = SystemCoreClock  ;
//	uint32_t b = 0 ;
    return 0 ;
}
