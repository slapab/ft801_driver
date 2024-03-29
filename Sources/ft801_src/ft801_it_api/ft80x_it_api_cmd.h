#ifndef _FT80X_IT_API_CMD_H_
#define _FT80X_IT_API_CMD_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


/**
*   This function must be called on each new command list to send. It copies the
*   destination address to the first three bytes in the ring buffer.
*   
*   @param addr the destination address ( which points to the specific chip mem.
*           location )
*/
void ft801_api_cmd_prepare_it( uint32_t addr );

/**
*   This function appends CMD command to the buffer.
*
*   @return false if there is no place in the buffer
*/
bool ft801_api_cmd_append_it( const uint32_t data );


void ft801_api_cmd_flush_it(void) ;


// ######### CHIP SPECIFIC WIDGETS API ##########

void ft801_api_cmd_spinner_it(
    int16_t x,
    int16_t y,
    uint16_t style,
    uint16_t scale
) ;


size_t ft801_api_cmd_text_it(
    int16_t x,
    int16_t y,
    int16_t font,
    uint16_t options,
    const char* str
);

    
void ft801_api_cmd_number_it( 
    int16_t x, 
    int16_t y, 
    int16_t font, 
    uint16_t options, 
    int32_t n
); 
    
void ft801_api_cmd_keys_it(
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    int16_t font, 
    uint16_t options, 
    const char* str
); 

void ft801_api_cmd_fgcolor_it( const uint32_t color ) ;
void ft801_api_cmd_bgcolor_it( const uint32_t color ) ;

    
    
void ft801_api_cmd_button_it( 
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    int16_t font, 
    uint16_t options, 
    const char* str
);

    
void ft801_api_cmd_track_it( 
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    int16_t tag
);



void ft801_api_cmd_memwrite_it(
    uint32_t addr,
    const uint8_t * const ptr,
    uint32_t num
);
    
    
void ft801_api_cmd_memwrite_16_it(
    const uint32_t addr,
    const uint16_t value
);
    
    
void ft801_api_cmd_slider_it(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t options,
    uint16_t val,
    uint16_t range
) ;



void ft801_api_cmd_gauge_it(
    int16_t x,
    int16_t y,
    int16_t r,
    uint16_t options,
    uint16_t major,
    uint16_t minor,
    uint16_t val,
    uint16_t range
) ;



void ft801_api_cmd_scrollbar_it(
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    uint16_t options, 
    uint16_t val, 
    uint16_t size, 
    uint16_t range
);


void ft801_api_cmd_gradcolor_it( uint32_t c ) ;



void ft801_api_cmd_progress_it( 
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    uint16_t options, 
    uint16_t val, 
    uint16_t range
); 



void ft801_api_cmd_toggle_it(
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t font, 
    uint16_t options, 
    uint16_t state, 
    const char* str
);




#endif
