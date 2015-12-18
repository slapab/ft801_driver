#include "ft80x_it_api_cmd.h"
#include "ft80x_it_ringbuffer.h"

#include "ft801_gpu.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


// ###### DECLARE SHARED API FUNCTIONS ######

// FROM ft80x_it_api.c/h
void ft80x_it_start_tx_cmds( size_t rb_fullness );
bool ft80x_it_reset( bool force );
uint16_t ft80x_spi_read_16bits( const uint32_t addr );





// ##### DECLARATION OF LOCAL STATIC FUNCTIONS #####
static void _cmd_append_str( const char * str ) ;



// ##### DECLARE LOCAL (STATIC) GLOBALS #####
static size_t reg_cmd_write_val ;




// NOTE : this function breaks currently working transmission !
// NOTE : this function not quaranties that CMD FIFO inside ft80x chip is empty,
//          it just assumes that.
void ft801_api_cmd_prepare_it( uint32_t addr )
{
    // reset state of it_engine
    ft80x_it_reset(true);
    // clear internal ringbuffer
    ft80x_it_ring_buffer_reset();
    
    // read the current offset of chip's CMD Circular buffer
    reg_cmd_write_val = ft80x_spi_read_16bits( REG_CMD_WRITE ) ;
    
    // update write address
    addr += reg_cmd_write_val ;
    addr |= 0x00800000 ; // set to notify that this is write addr
    // store the three bytes ( MSB -1, MSB-2, LSB ) in the ring buffer
    ft80x_it_ring_buffer_append((uint8_t)(addr>>16));
    ft80x_it_ring_buffer_append((uint8_t)(addr>>8));
    ft80x_it_ring_buffer_append((uint8_t)addr);
}

bool ft801_api_cmd_append_it( const uint32_t data )
{
    
    // copy bytes into buffer in the little - endian format
    ft80x_it_ring_buffer_append_32b_ld( data );
    
    //ft80x_it_start_tx_cmds(0);
       
    return true;
}
\
void ft801_api_cmd_flush_it(void)
{   
    // Calculate index for chip's cmds fifo:
    
    // get the number of commands stored in ringbuffer
    size_t fullness = ft80x_it_ring_buffer_fullness();
    // Prepare value for REG_CMD_WRITE
    fullness = ( fullness - 3)/*dicard 3B for address*/ + reg_cmd_write_val ;
    // handle of chip cmd's fifo size
    fullness %= FT_CMDFIFO_SIZE;
    
    
    // pass this value and start sending cmds
    ft80x_it_start_tx_cmds( fullness ) ;
}


void ft801_api_cmd_text_it(
    int16_t x,
    int16_t y,
    int16_t font,
    uint16_t options,
    const char* str
)
{
    if ( NULL == str )
        return ;
    
    
    // append into buffer right command
    ft801_api_cmd_append_it(CMD_TEXT);
    
    uint32_t tmp ;
    
    // wrap x, y position
    tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap font and the option arguments
    tmp = ((uint32_t)options << 16) | (uint32_t)font ;
    ft801_api_cmd_append_it(tmp) ;
    
    // append the string to the buffer
    _cmd_append_str( str );
    
}





void ft801_api_cmd_spinner_it(
    int16_t x,
    int16_t y,
    uint16_t style,
    uint16_t scale
)
{
    ft801_api_cmd_append_it(CMD_SPINNER) ;
    
    // wrap the value of x and y
    uint32_t tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap style and scale
    tmp = ((uint32_t)scale << 16) | (uint32_t)style ;
    ft801_api_cmd_append_it(tmp) ;
}







void ft801_api_cmd_number_it( 
    int16_t x, 
    int16_t y, 
    int16_t font, 
    uint16_t options, 
    int32_t n
)
{
    ft801_api_cmd_append_it(CMD_NUMBER);
    
    uint32_t tmp ;
    
    // wrap x, y position
    tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap font and the option
    tmp = ((uint32_t)options << 16) | (uint32_t)font ;
    ft801_api_cmd_append_it(tmp) ;
    
    // put the number
    ft801_api_cmd_append_it((uint32_t)n);
}







void ft801_api_cmd_keys_it(
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    int16_t font, 
    uint16_t options, 
    const char* str
)
{
    if ( NULL == str )
        return ;
    
    
    // append into buffer right command
    ft801_api_cmd_append_it(CMD_KEYS);
    
    uint32_t tmp ;
    
    // wrap x, y position
    tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap width and height
    tmp = ((uint32_t)h << 16) | (uint32_t)w ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap font and the option
    tmp = ((uint32_t)options << 16) | (uint32_t)font ;
    ft801_api_cmd_append_it(tmp) ;
    
    // append the string to the buffer
    _cmd_append_str( str );
}



void ft801_api_cmd_fgcolor_it( const uint32_t color )
{
    ft801_api_cmd_append_it(CMD_FGCOLOR);
    ft801_api_cmd_append_it(color);
}




void ft801_api_cmd_bgcolor_it( const uint32_t color )
{
    ft801_api_cmd_append_it(CMD_BGCOLOR);
    ft801_api_cmd_append_it(color);
}





void ft801_api_cmd_button_it( 
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    int16_t font, 
    uint16_t options, 
    const char* str
)
{
    if ( NULL == str )
        return ;
    
    
    // append into buffer right command
    ft801_api_cmd_append_it(CMD_BUTTON);
    
    uint32_t tmp ;
    
    // wrap x, y position
    tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap width and height
    tmp = ((uint32_t)h << 16) | (uint32_t)w ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap font and the option
    tmp = ((uint32_t)options << 16) | (uint32_t)font ;
    ft801_api_cmd_append_it(tmp) ;
    
    // append the string to the buffer
    _cmd_append_str( str );

}






void ft801_api_cmd_track_it( 
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    int16_t tag
)
{
    ft801_api_cmd_append_it(CMD_TRACK);
    
    uint32_t tmp ;
    // wrap x, y position
    tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap w, h position
    tmp = ((uint32_t)h << 16) | (uint32_t)w ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap the tag
    tmp = (0U << 16) | (uint32_t)tag ;
    ft801_api_cmd_append_it(tmp) ;
}








// ###### LOCAL HELP FUNCTIONS ######

void _cmd_append_byte( const uint8_t data )
{
    ft80x_it_ring_buffer_append( data );
}



void _cmd_append_str( const char * str )
{

    size_t str_len = strlen( str ) + 1; // +1 for null char
    size_t str_padd = 0;
    
    if ( 0 < (str_len % 4) )
        str_padd = 4 - (str_len % 4 );
    
    
    // put string into buffer
    ft80x_it_ring_buffer_appendBuff( str, str_len );
    
//    for( uint8_t* c = (uint8_t*)str; *c ; ++c )
//    {
//       ft80x_it_ring_buffer_append( *c ) ;
//    }
    // append the NULL character
   // ft801_api_cmd_append_it ('\0') ;
    
    // append the padding bytes
    for ( size_t i = 0 ; i < str_padd; ++i )
    {
        ft80x_it_ring_buffer_append(0) ;
    }        
    
}
