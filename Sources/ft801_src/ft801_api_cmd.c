#include <stddef.h>
#include "ft801_api_cmd.h"
#include "ft801_gpu.h"

#include "spi.h" // todo -> remove this dependecy


struct ft801_cmd_descr_t
{
    uint32_t  m_startAddr ;   // RAM_CMD address in the FT801
    uint32_t  m_buffSize ;    // size of buffer
    uint8_t*  m_pBuff ;       // pinter to the beggining of buffer for DL commands
    uint8_t*  m_pNextPtr ;    // points to the next free element in the buffer
} ;


static struct ft801_cmd_descr_t _cmd_descr;




void ft801_api_cmd_prepare(
        const uint32_t cmd_addr,
        uint8_t * buff,
        const int32_t buff_size )
{
    if ( 0 < buff_size )
    {
        _cmd_descr.m_pBuff = buff ;
        _cmd_descr.m_buffSize = (uint32_t)buff_size ;
    }
    
    // points to first free place after address
    _cmd_descr.m_pNextPtr = _cmd_descr.m_pBuff + 3 ;
    
    _cmd_descr.m_startAddr = cmd_addr ;
    
}



// LOCAL API: append 8Bit data to the buffer
// CAUTION ! NEVER TESTED
static uint32_t _cmd_append( const uint8_t data )
{
    // calculate used space for commands ( discard first 3 bytes used for address )
    uint32_t used_space = _cmd_descr.m_pNextPtr - (_cmd_descr.m_pBuff+3) ;
    
    // calculate buffer size (discard first 3 bytes used for address )
    uint32_t buff_cmd_size = _cmd_descr.m_buffSize-3;
    
    if ( used_space >= buff_cmd_size )
        return 0 ;

    // append to the buffer
    *(_cmd_descr.m_pNextPtr) = data ;
    ++ _cmd_descr.m_pNextPtr ;
    
    // return remained free space in the buffer
    return buff_cmd_size - used_space - 1;
    
}
\

// to do here, need allign str to the fully 4 bytes
static void _cmd_append_str( const char * str )
{
    // calculate used space for commands ( discard first 3 bytes used for address )
    uint32_t used_space = _cmd_descr.m_pNextPtr - (_cmd_descr.m_pBuff+3) ;
    
    // calculate buffer size (discard first 3 bytes used for address )
    uint32_t buff_cmd_size = _cmd_descr.m_buffSize-3;
    
    uint32_t free_space = buff_cmd_size - used_space ;
    
    uint8_t * pt = _cmd_descr.m_pNextPtr ;
    for( size_t i = 0 ; (str[i] != '\0') && ( 0 < free_space) ; ++i)
    {
        *pt = str[i] ;
        ++pt ;
    }
    // append the NULL character
    
    
}


bool ft801_api_cmd_append( const uint32_t data )
{
    // calculate used space for commands ( discard first 3 bytes for address )
    uint32_t used_space = _cmd_descr.m_pNextPtr - (_cmd_descr.m_pBuff+3) ;
    used_space >>= 2 ; // divide by 4 -> to calculate used space for full commands
    
    uint32_t buff_cmd_size = _cmd_descr.m_buffSize-3; // discard 3 bytes that are already used for address, 
    buff_cmd_size >>= 2 ; // divide by 4 to calculate total number of full command in the buffer
    

    // check if is free space in the buffer to store this command
    if( used_space >= buff_cmd_size )
    {
        return false ;
    }
    
    
    uint8_t * pt = _cmd_descr.m_pNextPtr ;
    
    // reserve space in the buffer
    _cmd_descr.m_pNextPtr += 4;
    
    // copy bytes into buffer in the little - endian format
    *pt = (uint8_t)data ;
    ++pt ;
    *pt = (uint8_t)(data >> 8) ;
    ++pt ;
    *pt = (uint8_t)(data >> 16) ;
    ++pt ;
    *pt = (uint8_t)(data >> 24) ;
    
    return true ;
}




uint32_t ft801_api_cmd_flush( void )
{
    uint32_t maxSize = _cmd_descr.m_pNextPtr - _cmd_descr.m_pBuff ;
    
    if ( NULL == _cmd_descr.m_pBuff )
        return 2 ;
    
    if ( 0 == maxSize )
        return 1 ;

    uint16_t cmd_wr_reg ;
    uint16_t cmd_rd_reg ;
    do
    {   
        cmd_rd_reg = ft801_spi_rd16( REG_CMD_READ ) ;
        cmd_wr_reg = ft801_spi_rd16( REG_CMD_WRITE ) ;
    } while ( cmd_wr_reg != cmd_rd_reg) ;
    
    // update write address in the buffer (3 bytes), MSB is first
    uint32_t new_addr = _cmd_descr.m_startAddr + cmd_wr_reg;
    new_addr |= 0x00800000 ;   // set bit to notify that this is write addr
    
    uint8_t * pt = _cmd_descr.m_pBuff ;
    *pt = (uint8_t)(new_addr >> 16) ;
    ++pt ;
    *pt = (uint8_t)(new_addr >> 8) ;
    ++pt ;
    *pt = (uint8_t)new_addr ;
    
    // Call SPI sending function
    spi_write_stream( _cmd_descr.m_pBuff, maxSize );
    
    // calculate how many commands was sent ( discard 3B for address )
    cmd_wr_reg += (maxSize - 3);
    cmd_wr_reg %= FT_CMDFIFO_SIZE ; // handle here overlapping ( local variable is 16 bit)
    // tell the GPU to process CMDs
    ft801_spi_mem_wr16(REG_CMD_WRITE, cmd_wr_reg) ;
    
    // reset descriptor state
    _cmd_descr.m_pNextPtr = _cmd_descr.m_pBuff + 3 ;
    
    return 0 ;
}












// todo check available place in the buffer
// if is not enough then flush buffer and append to the empty buffer
// todo add the same functionality to the _append()
void ft801_api_cmd_spinner(
        int16_t x,
        int16_t y,
        uint16_t style,
        uint16_t scale
)
{
    // todo ... check of free space
    
    
    ft801_api_cmd_append(CMD_SPINNER) ;
    
    // wrap the value of x and y
    uint32_t tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append(tmp) ;
    
    // wrap style and scale
    tmp = ((uint32_t)scale << 16) | (uint32_t)style ;
    ft801_api_cmd_append(tmp) ;
    
}



void ft801_api_cmd_text(
    int16_t x,
    int16_t y,
    int16_t font,
    uint16_t options,
    const char* str
)
{
    
    if ( NULL == str )
        return ;
    
    
    ft801_api_cmd_append(CMD_TEXT);
    
    uint32_t tmp ;
    
    // wrap x, y position
    tmp = ((uint32_t)y << 16) | (uint32_t)x ;
    ft801_api_cmd_append(tmp) ;
    
    // wrap font and the option
    tmp = ((uint32_t)options << 16) | (uint32_t)font ;
    ft801_api_cmd_append(tmp) ;
    
    // append the string to the buffer
    // todo here
    // finally add the byte
}


