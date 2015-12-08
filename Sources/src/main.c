#include <stddef.h>
#include "stm32f4xx.h"
#include "spi.h"

#include "spi_with_interrupts.h"

#include <signal.h>


#include "ft801_gpu.h"    // FT801 registers
#include "ft801_api.h"    // include api functions
#include "gpio_exti.h"      // init the gpio - exti -> for gpu interrupt pin

#include "ringBuffer_api.h"

#include <assert.h>

//#define _EXAMPLE_DL_TWO_BALLS
//#define _EXAMPLE_CMD
#define _EXAMPLE_TAG_TRACK
#define _CALIBRATE_TOUCH


// Declare the descryptor for ring buffer
rbd_t rb_spi_rd_descr ;
rbd_t rb_spi_wr_descr ;
// declare the memory for ring buffer
volatile uint8_t pSPI_buff_rd[16]; // read ring buffer
volatile uint8_t pSPI_buff_wr[16]; // write ring buffer

volatile size_t cnt = 0 ;

volatile sig_atomic_t gpu_int ;

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

// #### EXTI FROM FT801 GPU IRQ PIN ####
void EXTI4_IRQHandler(void)
{
    // test if was from GPU IT pin
    if ( (EXTI->PR & EXTI_PR_PR4) )
    {
        // clear flag in the exti register
        EXTI->PR |= EXTI_PR_PR4;
        
        gpu_int = 1;
    }
    
   
    // clear nvic flag
    NVIC_ClearPendingIRQ(EXTI4_IRQn);
}




void sleep( uint32_t delay )
{
    volatile int i = 0;
    for ( uint32_t i = 0 ; i < 1600000; ++i ) {}
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
    
    // Configure the NVIC and IRQs
    NVIC_ClearPendingIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn) ;
    
    
    // Configure the exti for gpu interrupt pin
    ft801_gpu_exti_conf() ;
    
    // init the SPI
    gpu_init_spi() ;
    
    
//    NVIC_EnableIRQ(SPI4_IRQn) ;
//    ft801_spi_hcmd_wr_it(FT_HOST_CMD_CLKEXT, rb_spi_wr_descr) ;
    
    
    
//    NVIC_ClearPendingIRQ(SPI4_IRQn) ;
    
//    cnt = 0 ;
    
    // init sequence
    ft801_spi_host_cmd( FT_CLKEXT ) ;
    ft801_spi_host_cmd( FT_CLK48M ) ;
    ft801_spi_host_cmd( FT_ACTIVE ) ;
    
    ft801_spi_host_cmd( FT_CORERST ) ;
    
    sleep(2) ;
    
    
    uint8_t ret = 0 ;
    ret = ft801_spi_rd8( REG_ID );
    
    
    
    if ( ret == 0x7C )
    {
        ft801_api_init_lcd() ;

        if ( false == ft801_api_is_enabled() )
        {
            ft801_api_enable_lcd(true);
        }
        
        // enable external interrupts
        ft801_api_enable_it_src(FT_INT_CMDEMPTY /*| FT_INT_TAG*/);
        ft801_api_enable_it_pin(true);
        // clear any pending flags
        ft801_api_read_it_flags() ;
        
        // set the backlight pwm duty 0-128
        ft801_spi_mem_wr8(REG_PWM_DUTY, 25) ;
        
        
        
#ifdef _EXAMPLE_DL_TWO_BALLS        
        
        uint32_t psize = 16*20 ;
        uint32_t x1 = 16+(psize/16);
        uint32_t y1 = 16+(psize/16);
        uint32_t x2 = 464-(psize/16);
        uint32_t y2 = 256-(psize/16);
        
        // push draw dl to the macro0 and macro1
        ft801_spi_mem_wr32(REG_MACRO_0, VERTEX2II(x1, y1, 0, 0));
        ft801_spi_mem_wr32(REG_MACRO_1, VERTEX2II(x2, y2, 0, 0));
        
        uint32_t dl_buffer[20] ;
        
        // create dl
        
        {
        ft801_api_dl_prepare(FT_RAM_DL, &dl_buffer[0], sizeof(dl_buffer)/sizeof(dl_buffer[0]) );
        ft801_api_dl_append(CLEAR(1, 1, 1)) ;
        ft801_api_dl_append(POINT_SIZE(psize));
        ft801_api_dl_append(BEGIN(FT_POINTS)) ;
        ft801_api_dl_append(COLOR_RGB(160, 22, 22));
        ft801_api_dl_append(MACRO(0)); // vertix
        ft801_api_dl_append(COLOR_RGB(160, 255, 255)) ;
        ft801_api_dl_append(COLOR_A( 80 ));
        ft801_api_dl_append( MACRO(1) ); //vertix 
        ft801_api_dl_append(END()) ;
        ft801_api_dl_append(DISPLAY()) ;
        ft801_api_dl_flush();
            

    
        } 
        ft801_spi_mem_wr8(REG_DLSWAP, FT_DLSWAP_FRAME); // swap list
        
        int32_t mov1 = 2 ;
        int32_t movy1 = 2 ;
        
        int32_t movx2 = -2 ;
        int32_t movy2 = -2 ;
        
        // example which using MACRO's in the GPU to moving two balls
        for ( ;; )
        {
            
            sleep(2);
            
            if ( (x1 >= (480-(psize/16))) || (x1 <= (0+(psize/16))) )
                mov1 *= -1 ;
    
            x1 += mov1;
            
            
            if ( (y1 > (272-(psize/16))) || (y1 < ( 0 + (psize/16))) )
                    movy1 *= -1 ;
                
            y1 += movy1 ;
            
            
            if ( (x2 >= (480-(psize/16))) || (x2 <= (0+(psize/16))) ) 
                movx2 *= -1 ;
            
            x2 += movx2 ;
           
            if ( (y2 > (272-(psize/16))) || (y2 < ( 0 + (psize/16))) )
                    movy2 *= -1 ;
                
            y2 += movy2 ;
            
            
            ft801_spi_mem_wr32(REG_MACRO_0, VERTEX2II(x1, y1, 0, 0));
            ft801_spi_mem_wr32(REG_MACRO_1, VERTEX2II(x2, y2, 0, 0));
        }
#endif

        
#ifdef _EXAMPLE_CMD
     
        uint8_t cmd_buffer[3+(4*60)] ; // buffer for 30 commands ( 3 for address)
        
        ft801_api_cmd_prepare(FT_RAM_CMD, cmd_buffer, sizeof(cmd_buffer)/sizeof(cmd_buffer[0]));
//        ft801_api_cmd_append(CMD_DLSTART) ;
//        ft801_api_cmd_append(CLEAR_COLOR_RGB(0, 230, 255));
//        ft801_api_cmd_append(CLEAR(1, 1, 1)) ;
//        ft801_api_cmd_append(POINT_SIZE(320));
//        ft801_api_cmd_append(BEGIN(FT_POINTS)) ;
//        ft801_api_cmd_append(COLOR_RGB(160, 22, 22));
//        ft801_api_cmd_append(VERTEX2II(32, 32, 0, 0)); // vertix
//        ft801_api_cmd_append(END()) ;
//        ft801_api_cmd_append(DISPLAY()) ;
//        ft801_api_cmd_append(CMD_SWAP);

          // spinner example
//        ft801_api_cmd_append(CMD_DLSTART) ;
//        ft801_api_cmd_append(CLEAR_COLOR_RGB(50, 50, 55));
//        ft801_api_cmd_append(CLEAR(1, 1, 1)) ;
//        ft801_api_cmd_spinner(240,150, 1,0);
        
        // calibrate the touchscreen driver
        //ft801_api_cmd_append(CMD_CALIBRATE);
        //ft801_api_cmd_append(CMD_SWAP);
        
        // example with text
        ft801_api_cmd_append(CMD_DLSTART) ;
        ft801_api_cmd_append(CLEAR_COLOR_RGB(50, 50, 55));
        ft801_api_cmd_append(CLEAR(1, 1, 1)) ;
        ft801_api_cmd_append(COLOR_RGB(0,100,250));
        ft801_api_cmd_text(240,136,30,FT_OPT_CENTER,"Slawomir Pabian") ;
        ft801_api_cmd_fgcolor(COLOR_RGB(0,150,10) );
        ft801_api_cmd_append(COLOR_RGB(255,255,255));
        ft801_api_cmd_keys(240,156,100,30,26,FT_OPT_FLAT,"12345");
        ft801_api_cmd_number(240,190,30,FT_OPT_SIGNED,-12) ;
        ft801_api_cmd_append(DISPLAY()) ;
        ft801_api_cmd_append(CMD_SWAP);
        ft801_api_cmd_flush();
       
//        sleep(1);
//        ft801_spi_mem_wr8(REG_DLSWAP, FT_DLSWAP_FRAME); // swap list 
        
#endif

        
#ifdef _CALIBRATE_TOUCH
         uint8_t cmd_buffer[3+(4*20)] ; // buffer for 30 commands ( 3 for address)
        
        ft801_api_cmd_prepare(FT_RAM_CMD, cmd_buffer, sizeof(cmd_buffer)/sizeof(cmd_buffer[0]));
        ft801_api_cmd_append(CMD_DLSTART) ;
        ft801_api_cmd_append(CLEAR(1,1,1));
        ft801_api_cmd_append(CMD_CALIBRATE);
        //ft801_api_cmd_append(DISPLAY()) ;
        ft801_api_cmd_append(CMD_SWAP);
        ft801_api_cmd_flush();
#endif
        
        while(1){
            if ( gpu_int == 1 )
            {
                
                gpu_int = 0 ;
                
                // read flags
                uint8_t fl = ft801_api_read_it_flags() ;
                if ( fl & FT_INT_CMDEMPTY ) {
                    break ;
                }
                
            }
        }
        
#ifdef _EXAMPLE_TAG_TRACK
        uint8_t cmd_buffer2[3+(4*60)] ; // buffer for 30 commands ( 3 for address)
        
        ft801_api_cmd_prepare(FT_RAM_CMD, cmd_buffer2, sizeof(cmd_buffer)/sizeof(cmd_buffer[0]));
        // example with text
        ft801_api_cmd_append(CMD_DLSTART) ;
        //ft801_api_cmd_append(CLEAR_COLOR_RGB(50, 50, 55));
        //ft801_api_cmd_append(CLEAR(1, 1, 1)) ;
        
        ft801_api_cmd_append( CLEAR_COLOR_RGB(5, 45, 110) ); 
        ft801_api_cmd_append( COLOR_RGB(255, 168, 64) ); 
        ft801_api_cmd_append( CLEAR(1 ,1 ,1) ); 
        ft801_api_cmd_append( BEGIN(FT_RECTS) ); 
        ft801_api_cmd_append( VERTEX2F(60 * 16,50 * 16) ); 
        ft801_api_cmd_append( VERTEX2F(100 * 16,62 * 16) ); 
        ft801_api_cmd_append( COLOR_RGB(255, 0, 0) ); 
        ft801_api_cmd_append( VERTEX2F(60 * 16,50 * 16) ); 
        ft801_api_cmd_append( VERTEX2F(80 * 16,62 * 16) ); 
        ft801_api_cmd_append( COLOR_MASK(0 ,0 ,0 ,0) ); 
        ft801_api_cmd_append( TAG(1) ); 
        ft801_api_cmd_append( VERTEX2F(60 * 16,50 * 16) ); 
        ft801_api_cmd_append( VERTEX2F(100 * 16,62 * 16) ); 
        ft801_api_cmd_track(60 * 16, 50 * 16, 40, 12, 1); 
        
        ft801_api_cmd_append(DISPLAY()) ;
        ft801_api_cmd_append(CMD_SWAP);
        ft801_api_cmd_flush();
        
        // enable TAG update interrupt
        ft801_api_enable_it_src(FT_INT_CMDEMPTY | FT_INT_TAG);
        while(1){
            if ( gpu_int == 1 )
            {
                
                gpu_int = 0 ;
                
                // read flags
                uint8_t fl = ft801_api_read_it_flags() ;
                
            }
        }
        
#endif
        
    
    }        


    
	SystemCoreClockUpdate();
	uint32_t a = SystemCoreClock  ;
//	uint32_t b = 0 ;
    return 0 ;
}
