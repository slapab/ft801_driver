#ifndef _FT801_API_CMD_H_
#define _FT801_API_CMD_H_

#include <stdint.h>
#include <stdbool.h>

/*
*   API for using CMD ( COMMAND List )
*/


/**
*   This is using for setting the buffer and its size. The api will use this.
*   After calling this function each calls to the ... will send 4 data bytes
*   in the next location in GPU_DL.
*
*   @param cmd_addr     address where RAM_CMD begins
*   @param *buff        pointer to the buffer where will be stored all
*                       CMD commands.
*           @note   this api provides right byte order while storing an address 
*                   and commands in the buffer.
*
*   @param buff_size    the size of buffer #buff. If this value is < 0 then
*                       calls to this function just resets the internal state.
*           @note   Please be informed that each command is 4 byte length and 
*                   address is 3 bytes length.
*/
void ft801_api_cmd_prepare( const uint32_t cmd_addr, uint8_t * buff, const int32_t buff_size ) ;

/**
*   This function appends CMD command to the buffer.

*   @return false if there is no place in the buffer
*/
bool ft801_api_cmd_append( const uint32_t data );

/**
*   This function sends data from internal buffer to the GPU.
*   
*   @retval 0 if data was successfully sent
*   @retval 1 if buffer was empty and nothing was sent
*   @retval 2 if buffer was not specified
*
*   @warning Sending is perfomed in blocking way.
*/
uint32_t ft801_api_cmd_flush( void ) ;









// ######### CHIP SPECIFIC WIDGETS API ##########

void ft801_api_cmd_spinner(
    int16_t x,
    int16_t y,
    uint16_t style,
    uint16_t scale
) ;


void ft801_api_cmd_text(
    int16_t x,
    int16_t y,
    int16_t font,
    uint16_t options,
    const char* str
);

    
void ft801_api_cmd_number( 
    int16_t x, 
    int16_t y, 
    int16_t font, 
    uint16_t options, 
    int32_t n
); 
    
void ft801_api_cmd_keys(
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    int16_t font, 
    uint16_t options, 
    const char* s
); 

void ft801_api_cmd_fgcolor( const uint32_t color ) ;
void ft801_api_cmd_bgcolor( const uint32_t color ) ;

    
    
void cmd_button( 
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    int16_t font, 
    uint16_t options, 
    const char* s
);


#endif
