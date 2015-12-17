#ifndef _FT80X_IT_API_CMD_H_
#define _FT80X_IT_API_CMD_H_

#include <stdint.h>
#include <stdbool.h>

/**
*   This function appends CMD command to the buffer.

*   @return false if there is no place in the buffer
*/
bool ft801_api_cmd_append_it( const uint32_t data );



// ######### CHIP SPECIFIC WIDGETS API ##########

void ft801_api_cmd_spinner_it(
    int16_t x,
    int16_t y,
    uint16_t style,
    uint16_t scale
) ;


void ft801_api_cmd_text_it(
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
\
#endif