#include <stdint.h>         // for uintX_t types
#include "spi.h"
#include "ft801_gpu.h"

// forward declarations:
void ft801_api_ctouch_adjust( void );
void ft801_api_enable_lcd( bool enable );


void ft801_api_init_lcd( void )
{
    /* define lcd params */
    const uint16_t FT_DispWidth = 480;
    const uint16_t FT_DispHeight = 272;
    const uint16_t FT_DispHCycle =  548;
    const uint16_t FT_DispHOffset = 43;
    const uint16_t FT_DispHSync0 = 0;
    const uint16_t FT_DispHSync1 = 41;
    const uint16_t FT_DispVCycle = 292;
    const uint16_t FT_DispVOffset = 12;
    const uint16_t FT_DispVSync0 = 0;
    const uint16_t FT_DispVSync1 = 10;
    const uint16_t FT_DispPCLK = 5;
    const uint16_t FT_DispSwizzle = 0;
    const uint16_t FT_DispPCLKPol = 1;
    
    
    
    // init sequence to start the GPU FT801
    // this configures the clock and sets to the active state
    ft801_spi_host_cmd( FT_CLKEXT ) ;
    ft801_spi_host_cmd( FT_CLK48M ) ;
    ft801_spi_host_cmd( FT_ACTIVE ) ;
    ft801_spi_host_cmd( FT_CORERST ) ;
    
    // some dleay 
    {
        volatile uint32_t tmp = 1000;
        for ( uint32_t i = 0 ; i < tmp ; ++i );
    }
    
    
    // Push this params to the FT core

    ft801_spi_mem_wr16( REG_HSIZE, FT_DispWidth) ;
    ft801_spi_mem_wr16( REG_HCYCLE, FT_DispHCycle) ;
    ft801_spi_mem_wr16( REG_HOFFSET, FT_DispHOffset) ;
    ft801_spi_mem_wr16( REG_HSYNC0, FT_DispHSync0) ;
    ft801_spi_mem_wr16( REG_HSYNC1, FT_DispHSync1) ;

    ft801_spi_mem_wr16( REG_VSIZE, FT_DispHeight) ;
    ft801_spi_mem_wr16( REG_VCYCLE, FT_DispVCycle) ;
    ft801_spi_mem_wr16( REG_VOFFSET, FT_DispVOffset) ;
    ft801_spi_mem_wr16( REG_VSYNC0, FT_DispVSync0) ;
    ft801_spi_mem_wr16( REG_VSYNC1, FT_DispVSync1) ;
    
    ft801_spi_mem_wr8( REG_SWIZZLE, FT_DispSwizzle) ;
    ft801_spi_mem_wr8( REG_PCLK_POL, FT_DispPCLKPol) ;
        
    ft801_spi_mem_wr8( REG_PCLK, FT_DispPCLK) ; // enable pll - whole gpu





    // send the transform matrix to ctouch registers
    // this will calibrate the the touch screen
    ft801_api_ctouch_adjust();
    
    // Turn on the LCD panel
    ft801_api_enable_lcd(true);
}


void ft801_api_enable_lcd( bool enable )
{
    uint8_t tmp ;
    
    // direction register
    tmp = ft801_spi_rd8(REG_GPIO_DIR) ;
    if ( true == enable )
    {    
        tmp |= ( 1 << 7 ) ;
        ft801_spi_mem_wr8(REG_GPIO_DIR, tmp);
    }
    
    // value register
    tmp = ft801_spi_rd8(REG_GPIO) ;
    if ( true == enable )
        tmp |= ( 1 << 7 );
    else
        tmp &= ~( 1 << 7 );
    
    ft801_spi_mem_wr8(REG_GPIO, tmp);
}


bool ft801_api_is_enabled( void )
{
    uint8_t tmp;
    
    tmp = ft801_spi_rd8(REG_GPIO_DIR) ;
    if ( !(tmp & (1<<7)) )
        return false ;
    
    tmp = ft801_spi_rd8(REG_GPIO) ;
    if ( !(tmp & (1<<7)) )
        return false ;
    
    
    return true ;
}




void ft801_api_enable_it_pin( bool enable )
{
    if( true == enable )
        ft801_spi_mem_wr8( REG_INT_EN, 1 );
    else
        ft801_spi_mem_wr8( REG_INT_EN, 0 ) ;
}


void ft801_api_enable_it_src( const uint8_t mask )
{
    ft801_spi_mem_wr8( REG_INT_MASK, mask );
}



void ft801_api_disable_it_src( const uint8_t mask )
{
    ft801_spi_mem_wr8( REG_INT_MASK,
            ft801_spi_rd8(REG_INT_MASK) & (~mask) );
}


uint8_t ft801_api_read_it_flags( void )
{
    return ft801_spi_rd8( REG_INT_FLAGS );
}



void ft801_api_ctouch_adjust( void )
{
    // This values are for Riverdi 4.3" Capacitive touchscreen
    uint32_t A = 25592 ;
    uint32_t B = 1637 ;
    uint32_t C = 4293890945 ;
    uint32_t D = 556 ;
    uint32_t E = 24377 ;
    uint32_t F = 4293991767 ;
    
    ft801_spi_mem_wr32(REG_CTOUCH_TRANSFORM_A, A);
    ft801_spi_mem_wr32(REG_CTOUCH_TRANSFORM_B, B);
    ft801_spi_mem_wr32(REG_CTOUCH_TRANSFORM_C, C);
    ft801_spi_mem_wr32(REG_CTOUCH_TRANSFORM_D, D);
    ft801_spi_mem_wr32(REG_CTOUCH_TRANSFORM_E, E);
    ft801_spi_mem_wr32(REG_CTOUCH_TRANSFORM_F, F);
    
}
