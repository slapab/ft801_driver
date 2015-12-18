#ifndef _FT80X_IT_API_H_
#define _FT80X_IT_API_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "ft80x_it_api_cmd.h"


typedef void (*pf_en_TypeDef)( bool ) ;
typedef uint16_t (*spi_rd_16_TypeDef)( const uint32_t );
typedef void (*pf_void_void)( void ) ;



bool ft80x_it_rountine(uint8_t * const dst );
bool ft80x_it_check(void);

void ft80x_it_api_init( 
    pf_en_TypeDef       fptr_spi_en,
    pf_en_TypeDef       fptr_spi_it_en,
    pf_void_void        fptr_spi_set_pending,
    spi_rd_16_TypeDef   fptr_spi_rd16
);

#endif
