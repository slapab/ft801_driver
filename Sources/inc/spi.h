#ifndef _GPU_SPI_H_
#define _GPU_SPI_H_

	#include <stdbool.h>
	
	void ft801_spi_enable( const bool enabled ) ;
	void gpu_init_spi(void) ;
    
    void gpu_spi_send( uint32_t reg ) ;
    
    void ft801_spi_hcmd_write( const uint32_t cmd );
    
    
    uint8_t ft801_spi_rd8( const uint32_t reg );
    void ft801_spi_mem_wr16( const uint32_t addr, const uint16_t data, const bool one_shot ) ;
    uint16_t ft801_spi_mem_rd16( const uint32_t addr ) ;
    
    
    
#endif
