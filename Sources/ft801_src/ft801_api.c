#include <stdint.h>         // for uintX_t types
#include "spi.h"
#include "ft801_gpu.h"

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
    
    // Push this params to the FT core
    ft801_spi_enable(true) ;
    ft801_spi_mem_wr16( REG_HSIZE, FT_DispWidth, false ) ;
    ft801_spi_mem_wr16( REG_HCYCLE, FT_DispHCycle, false ) ;
    ft801_spi_mem_wr16( REG_HOFFSET, FT_DispHOffset, false ) ;
    ft801_spi_mem_wr16( REG_HSYNC0, FT_DispHSync0, false ) ;
    ft801_spi_mem_wr16( REG_HSYNC1, FT_DispHSync1, false ) ;

    ft801_spi_mem_wr16( REG_VSIZE, FT_DispHeight, false ) ;
    ft801_spi_mem_wr16( REG_VCYCLE, FT_DispVCycle, false ) ;
    ft801_spi_mem_wr16( REG_VOFFSET, FT_DispVOffset, false ) ;
    ft801_spi_mem_wr16( REG_VSYNC0, FT_DispVSync0, false ) ;
    ft801_spi_mem_wr16( REG_VSYNC1, FT_DispVSync1, false ) ;
    
    ft801_spi_mem_wr16( REG_SWIZZLE, FT_DispSwizzle, false ) ;
    ft801_spi_mem_wr16( REG_PCLK_POL, FT_DispPCLKPol, false ) ;
        
    ft801_spi_mem_wr16( REG_PCLK, FT_DispPCLK, false ) ; // enable pll - whole gpu

    
    ft801_spi_enable(false) ;
}
