#ifndef _GPU_SPI_H_
#define _GPU_SPI_H_

	#include <stdbool.h>
	
	void ft801_spi_enable( const bool enabled ) ;
	void gpu_init_spi(void) ;
        
    void ft801_spi_host_cmd( uint32_t cmd );
    
    
    uint8_t ft801_spi_rd8( uint32_t addr );
    
    void ft801_spi_mem_wrStream( const uint32_t addr,
                            uint32_t * pBuff,
                            const uint32_t len );
    
    void ft801_spi_mem_wr32( const uint32_t addr,
                         const uint32_t data) ;
    void ft801_spi_mem_wr16( const uint32_t addr, const uint16_t data) ;
    void ft801_spi_mem_wr8( const uint32_t addr,
                        const uint8_t data ) ;
    uint16_t ft801_spi_rd16( uint32_t addr ) ;
    
    
    
#endif
