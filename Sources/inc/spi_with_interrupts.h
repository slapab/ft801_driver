#ifndef _SPI_WITH_INTERRUPTS_H_
#define _SPI_WITH_INTERRUPTS_H_

#include "ringBuffer_api.h"

void ft801_spi_hcmd_wr_it( const uint32_t cmd, const rbd_t spi_wr_descr );

#endif
