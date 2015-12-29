#include <stddef.h>
#include "stm32f4xx.h"
#include "spi.h"

#include "spi_with_interrupts.h"

#include <signal.h>


#include "ft801_gpu.h"    // FT801 registers
#include "ft801_api.h"    // include api functions
#include "gpio_exti.h"    // init the gpio - exti -> for gpu interrupt pin


#include "ft80x_it_api.h"
#include "ft80x_engine_it.h"

#include "my_tasks.h"

//#include "ringBuffer_api.h"

#include <assert.h>

//#define _EXAMPLE_DL_TWO_BALLS
//#define _EXAMPLE_CMD
//#define _EXAMPLE_TAG_TRACK
//#define _CALIBRATE_TOUCH

//#define _TEST_IT_API

#define _TEST_GPU_ENGINE


// Declare the descryptor for ring buffer
rbd_t rb_spi_rd_descr ;
rbd_t rb_spi_wr_descr ;
// declare the memory for ring buffer
volatile uint8_t pSPI_buff_rd[16]; // read ring buffer
volatile uint8_t pSPI_buff_wr[16]; // write ring buffer

volatile size_t cnt = 0 ;

volatile sig_atomic_t gpu_int ;

void enable_spi( bool enable )
{
    if ( true == enable )
        SPI4->CR1 |= SPI_CR1_SPE ;
    else
        SPI4->CR1 &= ~(SPI_CR1_SPE);
}

void enable_spi_interrupt( bool enable )
{
    NVIC_ClearPendingIRQ(SPI4_IRQn);
    if ( true == enable )
        NVIC_EnableIRQ(SPI4_IRQn) ;
    else
        NVIC_DisableIRQ(SPI4_IRQn) ;
}

void set_spi_pending_int(void)
{
    NVIC_SetPendingIRQ(SPI4_IRQn) ;
}

void SPI4_IRQHandler(void)
{
    // Receive
    if( READ_BIT(SPI4->SR, SPI_SR_RXNE) )
    {
        uint8_t tmp = SPI4->DR ;
        NVIC_ClearPendingIRQ(SPI4_IRQn) ;
    }
    
    
    // Transmit
    if ( READ_BIT( SPI4->SR, SPI_SR_TXE) )
    {
        if ( true == ft80x_it_rountine( (uint8_t*)&(SPI4->DR) ) )
        {
             NVIC_ClearPendingIRQ(SPI4_IRQn) ;
        }
    }
    
   
    
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
        
        ft80x_gpu_eng_it_rountine();
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
    
    
    //NVIC_EnableIRQ(SPI4_IRQn) ;

    
    
    
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
        // send the transform matrix to ctouch registers
        ft801_api_ctouch_adjust();
        
        ft801_api_init_lcd() ;
        
        if ( false == ft801_api_is_enabled() )
        {
            ft801_api_enable_lcd(true);
        }
        
        // enable external interrupts
        ft801_api_enable_it_src(FT_INT_CMDEMPTY | FT_INT_TAG | FT_INT_CONVCOMPLETE | FT_INT_TOUCH);
        ft801_api_enable_it_pin(true);
        // clear any pending flags
        ft801_api_read_it_flags() ;
        
        // set the backlight pwm duty 0-128
        ft801_spi_mem_wr8(REG_PWM_DUTY, 25) ;
        // enable the extended mode for touch-screen
        ft801_spi_mem_wr8(REG_CTOUCH_EXTENDED, 0) ;
        
        
        
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
         uint8_t cmd_buffer[3+(4*40)] ; // buffer for 30 commands ( 3 for address)
        
        ft801_api_cmd_prepare(FT_RAM_CMD, cmd_buffer, sizeof(cmd_buffer)/sizeof(cmd_buffer[0]));
        ft801_api_cmd_append(CMD_DLSTART) ;
        ft801_api_cmd_append(CLEAR(1,1,1));
        
        ft801_api_cmd_append(CMD_CALIBRATE);
        ft801_api_cmd_append(DISPLAY()) ;
        ft801_api_cmd_append(CMD_SWAP);
        ft801_api_cmd_flush();

        
        while(1){
            if ( gpu_int == 1 )
            {
                
                gpu_int = 0 ;
                
                // read flags
                uint8_t fl = ft801_api_read_it_flags() ;
                if ( fl & FT_INT_CMDEMPTY ) {
                    // after calibration display the calibrated registers:
                    // A-F
                    
                    ft801_api_cmd_append(CMD_DLSTART) ;
                    ft801_api_cmd_append(CLEAR(1,1,1));
                    ft801_api_cmd_number(0,0,25,0, ft801_spi_rd32( REG_CTOUCH_TRANSFORM_A )) ;
                    ft801_api_cmd_number(0,30,25,0, ft801_spi_rd32( REG_CTOUCH_TRANSFORM_B )) ;
                    ft801_api_cmd_number(0,60,25,0, ft801_spi_rd32( REG_CTOUCH_TRANSFORM_C )) ;
                    ft801_api_cmd_number(0,90,25,0, ft801_spi_rd32( REG_CTOUCH_TRANSFORM_D )) ;
                    ft801_api_cmd_number(0,120,25,0, ft801_spi_rd32( REG_CTOUCH_TRANSFORM_E )) ;
                    ft801_api_cmd_number(0,150,25,0, ft801_spi_rd32( REG_CTOUCH_TRANSFORM_F )) ;
                    ft801_api_cmd_append(DISPLAY()) ;
                    ft801_api_cmd_append(CMD_SWAP);
                    ft801_api_cmd_flush();
                    
                    
                    break ;
                }
                
            }
        }
#endif 
        
        
        
        
#ifdef _EXAMPLE_TAG_TRACK
        uint8_t cmd_buffer2[3+(4*60)] ; // buffer for 30 commands ( 3 for address)
        
        ft801_api_cmd_prepare(FT_RAM_CMD, cmd_buffer2, sizeof(cmd_buffer2)/sizeof(cmd_buffer2[0]));
        
        ft801_api_cmd_append(CMD_DLSTART) ;
        
        ft801_api_cmd_append( CLEAR_COLOR_RGB(5, 45, 110) );
        ft801_api_cmd_append( COLOR_RGB(255, 168, 64) );
        ft801_api_cmd_append( CLEAR(1 ,1 ,1) );
        ft801_api_cmd_append( TAG(1) );
        ft801_api_cmd_append( BEGIN(FT_POINTS) );
        ft801_api_cmd_append( POINT_SIZE(20 * 16) );
        ft801_api_cmd_append( VERTEX2F(80 * 16, 60 * 16) );
        //ft801_api_cmd_track(80 * 16, 60 * 16, 1, 1, 1);
        ft801_api_cmd_append( TAG(20) ) ;
        ft801_api_cmd_append( VERTEX2F(280 * 16, 60 * 16) );
        //ft801_api_cmd_track(280 * 16, 60 * 16, 1, 1, 1);
        ft801_api_cmd_append(END()) ;
       
        
        ft801_api_cmd_append(DISPLAY()) ;
        ft801_api_cmd_append(CMD_SWAP);
        ft801_api_cmd_flush();
        
        // enable TAG update interrupt
        ft801_api_enable_it_src(FT_INT_CMDEMPTY | FT_INT_TAG | FT_INT_CONVCOMPLETE);
        while(1){
            if ( gpu_int == 1 )
            {
                
                gpu_int = 0 ;
                
                // read flags
                uint8_t fl = ft801_api_read_it_flags() ;
                if ( fl & FT_INT_TAG )
                {
                    int t ;
                    if ( (t = ft801_spi_rd8( REG_TOUCH_TAG )) == 1 ) {
                        volatile int b = 0 ;
                        b = t;
                    }
                    else {
                        volatile int c = 1 ;
                        c = t;
                    }
                }
                else if ( fl & FT_INT_CONVCOMPLETE )
                {
                    volatile int d = 23;
                    d++ ;
                    int a = d ;
                    d-- ;
                }
            
            }
        }
        
#endif
       

#ifdef _TEST_IT_API
        
        ft80x_it_api_init( enable_spi, enable_spi_interrupt, set_spi_pending_int, ft801_spi_rd16 );
        
        
        ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
        
        ft801_api_cmd_append_it(CMD_DLSTART) ;
        ft801_api_cmd_append_it(CLEAR_COLOR_RGB(50, 50, 55));
        ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
        ft801_api_cmd_append_it(COLOR_RGB(0,100,250));
        ft801_api_cmd_text_it(240,136,30,FT_OPT_CENTER,"Slawomir Pabian to jest mistrz ;-)") ;

        ft801_api_cmd_append_it(DISPLAY()) ;
        ft801_api_cmd_append_it(CMD_SWAP);

        ft801_api_cmd_flush_it();


        while ( false == ft80x_it_check() );
        
        
        // the second screen
        sleep(23) ;
        sleep(23) ;
        sleep(23) ;
        
        ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
        
        ft801_api_cmd_append_it(CMD_DLSTART) ;
        ft801_api_cmd_append_it(CLEAR_COLOR_RGB(50, 50, 55));
        ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
        ft801_api_cmd_append_it(COLOR_RGB(50,50,250));
        ft801_api_cmd_text_it(240,136,30,FT_OPT_CENTER,"Master of masters!") ;

        ft801_api_cmd_append_it(DISPLAY()) ;
        ft801_api_cmd_append_it(CMD_SWAP);

        ft801_api_cmd_flush_it();
        
        while ( false == ft80x_it_check() );

        // try to change the brightness
//        sleep(23) ;
//        sleep(23) ;
//        ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
//        
//        //ft801_api_cmd_append_it(CMD_DLSTART) ;
//        uint8_t tab[1] = { 1 } ;
//        ft801_api_cmd_memwrite_it(REG_PWM_DUTY, tab , 1) ;
//        
//        //ft801_api_cmd_append_it(DISPLAY()) ;
//        //ft801_api_cmd_append_it(CMD_SWAP);

//        ft801_api_cmd_flush_it();
//        
//        while ( false == ft80x_it_check() );

        
#endif        


#ifdef _TEST_GPU_ENGINE

        // init it_api
        ft80x_it_api_init( enable_spi, enable_spi_interrupt, set_spi_pending_int, ft801_spi_rd16 );
        // init the gpu_engine api
        ft80x_gpu_eng_it_init() ;
        
        
        // create new task
        uint8_t tab[1] = { 0 } ;
        FT80xTask_TypeDef g_task1 ;
        g_task1.mfp_doing = task1_doing ;
        g_task1.mfp_painting = task1_painting;
        g_task1.mfp_gpu_it = task1_gpuit ;
        g_task1.mp_shared_data = tab ;
        g_task1.m_id = TASK_ID1 ;
        
        // new task two
        uint16_t tab2[5] = {0,0,0,0,64} ;
        FT80xTask_TypeDef g_task2 ;
        g_task2.mfp_doing = task2_doing ;
        g_task2.mfp_painting = task2_painting;
        g_task2.mfp_gpu_it = task2_gpuit ;
        g_task2.mp_shared_data = tab2 ;
        g_task2.m_id = TASK_ID2 ;
        
        
        // keyboard task
        uint16_t tab3[3] = {0,0,0} ;
        FT80xTask_TypeDef g_task3 ;
        g_task3.mfp_doing = keyboardTask_doing ;
        g_task3.mfp_painting = keyboardTask_painting;
        g_task3.mfp_gpu_it = keyboardTask_gpuit ;
        g_task3.mp_shared_data = tab3 ;
        g_task3.m_id = TASK_KEYBOARD ;
        
        // graph task
        uint16_t tab4[5] = {0,0,0,0,0} ;
        FT80xTask_TypeDef g_task4 ;
        g_task4.mfp_doing = graphExample_doing ;
        g_task4.mfp_painting = graphExample_painting;
        g_task4.mfp_gpu_it = graphExample_gpuit ;
        g_task4.mp_shared_data = tab4 ;
        g_task4.m_id = TASK_GRAPH ;
        
        
        // register tasks
        ft80x_gpu_eng_it_reg_task(&g_task1) ;
        ft80x_gpu_eng_it_reg_task(&g_task2) ;
        ft80x_gpu_eng_it_reg_task(&g_task3) ;
        ft80x_gpu_eng_it_reg_task(&g_task4) ;
        
        
        // set active task
        ft80x_gpu_eng_it_setActiveTask(TASK_ID1) ;
        
        while(1)
        {
            ft80x_it_check() ; // pooling the it_api
            ft80x_gpu_eng_it_looper() ; // pooling the gpu_engine
        }


#endif


    }        


    
	SystemCoreClockUpdate();
	uint32_t a = SystemCoreClock  ;
//	uint32_t b = 0 ;
    return 0 ;
}
