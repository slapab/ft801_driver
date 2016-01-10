#include "ft80x_it_api_cmd.h"
#include "ft80x_it_ringbuffer.h"

#include "ft801_gpu.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


// ###### DECLARE SHARED API FUNCTIONS ######

// FROM ft80x_it_api.c/h
void ft80x_it_cmds_append_finished( size_t rb_fullness );
void ft80x_it_cmds_start_tx( void ) ;
bool ft80x_it_cmds_reset( bool force );
uint16_t ft80x_spi_read_16bits( const uint32_t addr );





// ##### DECLARATION OF LOCAL STATIC FUNCTIONS #####
static size_t _cmd_append_str( const char * str ) ;
static void _cmd_append_buff( const uint8_t * const buff, const uint32_t size );




// ##### DECLARE LOCAL (STATIC) GLOBALS #####
static size_t reg_cmd_write_val ;    // holds value read from REG_CMD_WRITE before appending commands
static size_t bytes_in_transaction ; // how many cmd bytes was appended to the buffer





// NOTE : this function breaks currently working transmission !
// NOTE : this function not quaranties that CMD FIFO inside ft80x chip is empty,
//          it just assumes that.
void ft801_api_cmd_prepare_it( uint32_t addr )
{
    // reset state of it_engine
    ft80x_it_cmds_reset(true);
    // clear internal ringbuffer
    ft80x_it_ring_buffer_reset();
    
        bytes_in_transaction = 0 ;
    
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
    
        bytes_in_transaction += 4 ;
    
    ft80x_it_cmds_start_tx();
    return true;
}
\



void ft801_api_cmd_flush_it(void)
{       
    // Calculate index for chip's cmds fifo:
    size_t fullness = bytes_in_transaction ;
    // Prepare value for REG_CMD_WRITE
    fullness += reg_cmd_write_val ;
    // Handle of chip cmd's fifo size
    fullness %= FT_CMDFIFO_SIZE ;
    
    // pass this value and start sending cmds
    ft80x_it_cmds_append_finished( fullness ) ;
}




size_t ft801_api_cmd_text_it(
    int16_t x,
    int16_t y,
    int16_t font,
    uint16_t options,
    const char* str
)
{
    if ( NULL == str )
        return 0 ;
    
    
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
    return _cmd_append_str( str );
    
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
    tmp = 0U | (uint32_t)tag ;
    ft801_api_cmd_append_it(tmp) ;
}





void ft801_api_cmd_memwrite_it(
    const uint32_t addr,
    const uint8_t * const ptr,
    const uint32_t num
)
{
    // append to the buffer the the command
    ft801_api_cmd_append_it( CMD_MEMWRITE );
    
    // append the address in the memory ( register can be used too )
    ft801_api_cmd_append_it( addr ) ;
    
    // append the number of BYTES
    ft801_api_cmd_append_it( num ) ;
    
    // append the bytes -> with 4B aligment!
    _cmd_append_buff( ptr, num ) ;
}


// write 16 bit
void ft801_api_cmd_memwrite_16_it(
    const uint32_t addr,
    const uint16_t value
)
{
    // append to the buffer the the command
    ft801_api_cmd_append_it( CMD_MEMWRITE );
    
    // append the address in the memory ( register can be used too )
    ft801_api_cmd_append_it( addr ) ;
    
    // append the number of BYTES
    ft801_api_cmd_append_it( 2 ) ;
    
    // append the number of BYTES
    ft801_api_cmd_append_it( (uint32_t)value ) ;
}


void ft801_api_cmd_slider_it(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    uint16_t options,
    uint16_t val,
    uint16_t range
)
{
   // append to the buffer the the command
    ft801_api_cmd_append_it( CMD_SLIDER );
    
    uint32_t tmp ;
    // wrap x, y position
    tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap w, h parameters
    tmp = ((uint32_t)h << 16) | (uint32_t)w ;
    ft801_api_cmd_append_it( tmp ) ;
    
    // wrap options and val parameters
    tmp = ((uint32_t)val << 16) | (uint32_t)options ;
    ft801_api_cmd_append_it( tmp ) ;
    
    // wrap range parameter
    tmp = 0u | (uint32_t)range ;
    ft801_api_cmd_append_it( tmp ) ;
}



void ft801_api_cmd_gauge_it(
    int16_t x,
    int16_t y,
    int16_t r,
    uint16_t options,
    uint16_t major,
    uint16_t minor,
    uint16_t val,
    uint16_t range
)
{
    // append to the buffer the the command
    ft801_api_cmd_append_it( CMD_GAUGE );
    
    uint32_t tmp ;
    // wrap x, y position
    tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap w, h parameters
    tmp = ((uint32_t)options << 16) | (uint32_t)r ;
    ft801_api_cmd_append_it( tmp ) ;
    
    // wrap options and val parameters
    tmp = ((uint32_t)minor << 16) | (uint32_t)major ;
    ft801_api_cmd_append_it( tmp ) ;
    
    // wrap range parameter
    tmp = ((uint32_t)range << 16) | (uint32_t)val ;
    ft801_api_cmd_append_it( tmp ) ;
}



void ft801_api_cmd_scrollbar_it(
    int16_t x, 
    int16_t y, 
    int16_t w, 
    int16_t h, 
    uint16_t options, 
    uint16_t val, 
    uint16_t size, 
    uint16_t range
)
{
    // append to the buffer the the command
    ft801_api_cmd_append_it( CMD_SCROLLBAR );
    
    uint32_t tmp ;
    // wrap x, y position
    tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append_it(tmp) ;
    
    // wrap w, h parameters
    tmp = ((uint32_t)h << 16) | (uint32_t)w ;
    ft801_api_cmd_append_it( tmp ) ;
    
    // wrap options and val parameters
    tmp = ((uint32_t)val << 16) | (uint32_t)options ;
    ft801_api_cmd_append_it( tmp ) ;
    
    // wrap the size and range parameters
    tmp = ((uint32_t)range << 16) | (uint32_t)size ;
    ft801_api_cmd_append_it( tmp ) ;
}





// ###### LOCAL HELP FUNCTIONS ######

void _cmd_append_byte( const uint8_t data )
{
    ft80x_it_ring_buffer_append( data );
        bytes_in_transaction += 1;
}



static size_t _cmd_append_str( const char * str )
{

    size_t str_len = strlen( str ) + 1; // +1 for null char
    size_t str_padd = 0;
    
    if ( 0 < (str_len % 4) )
        str_padd = 4 - (str_len % 4 );
    
    
    // put string into buffer
    ft80x_it_ring_buffer_appendBuff( (const uint8_t *)str, str_len );
    
    
    // append the padding bytes       
    if ( str_padd > 0 )
        ft80x_it_ring_buffer_copyto(0, str_padd);
    
    
    bytes_in_transaction += str_len + str_padd ;
    
    return str_len ;
}



static void _cmd_append_buff( const uint8_t * const buff, const uint32_t size )
{
    // calculate the padding bytes
    size_t padd_no = size % 4; // this value is just temporary
    
    // calculate valid padding bytes
    padd_no = ( 0 < padd_no ) ? ( 4 - padd_no ) : 0 ;
    
    // put buffer into ring buffer
    ft80x_it_ring_buffer_appendBuff( buff, size );
    
    // put the padding bytes
    if ( padd_no > 0 )
        ft80x_it_ring_buffer_copyto(0, padd_no);
    
    
    // update internal counter
    bytes_in_transaction += size + padd_no ;
}
