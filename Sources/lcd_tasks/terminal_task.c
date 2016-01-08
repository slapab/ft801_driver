#include "terminal_task.h"
#include "FT801_gpu.h"
#include "lcd_tasks_ids.h"

#include "ft80x_it_api_cmd.h"
#include "ft80x_engine_it.h"


#include <stddef.h>
#include <string.h> // strlen


// forward declaration of local functions: 
static size_t _get_prev_line(void) ;
static size_t _get_next_line(void) ;





// internal buffer for storing lines
struct _container_t
{
    // define the area used to display tekst
    uint16_t m_area_w ;
    uint16_t m_area_h ;
 
    // buffer for storing the displayed tekst
    uint8_t * m_buff ;
    size_t m_size;
    size_t m_curr_pos ; // points to the next free space in buffer
    size_t m_start_line ; // the line from which it will be start printing
    
    
    // font widht and height // refer to the datasheet of the chip
    uint8_t m_font_w ;
    uint8_t m_font_h ;
    
    uint8_t m_chars ; // chars in line
    uint8_t m_lines ; // total line in given area
       
} ;

static uint8_t _string_buffer[2000] ;
static struct _container_t _thisData =
    {   
        // define the area used for displaying the
        440,
        100,
        
        // buffer params
        _string_buffer,
        2000, // size of the buffer
        0, // start from beggining
        0, // print from the beggingin
        
        // with, height for the font 18/19
        8,
        16,
        
        // other values computed by software
        0,
        0
    };

    
    

bool terminalTask_painting (void * const data)
{
    uint16_t * myData = data ;
    
//    // compute the needed params to handle text wraping on the screen
//    _thisData.m_lines = _thisData.m_area_h / _thisData.m_font_h ;
//    _thisData.m_chars = _thisData.m_area_w / _thisData.m_font_w ;
    
    
    
    // start creating the screen
    
    ft801_api_cmd_prepare_it(FT_RAM_CMD) ;
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    
    ft801_api_cmd_append_it(CLEAR(1,1,1));
    
    int16_t x_pos = 0 ;
    int16_t y_pos = 0 ;
    
    for ( int i = _thisData.m_lines; i >= 0 ; --i )
    {
        size_t len = ft801_api_cmd_text_it(x_pos, y_pos, 18, 0, (const char *)&_thisData.m_buff[ _thisData.m_start_line ]);
        
        if ( len == 0 ) break ;
        
        if ( (_thisData.m_start_line + len) < (_thisData.m_curr_pos-1))
        {
            // points to the new line
            _thisData.m_start_line += len ;
            
            // calcualte the new y pos, and check if area's boundary was riched
            y_pos += _thisData.m_font_h ; 
            if ( y_pos >= _thisData.m_area_h )
                break ;
        }
        else
        {
            break ;
        }
    }
    
    
    // draw the slider
    ft801_api_cmd_append_it(POINT_SIZE(18*16)) ;
    ft801_api_cmd_append_it(BEGIN(FT_POINTS)) ;
    ft801_api_cmd_append_it(COLOR_RGB(160,50,50)) ;
    ft801_api_cmd_append_it(TAG(10)) ;
    ft801_api_cmd_append_it(VERTEX2F(460*16, 100*16)) ; // up 
    ft801_api_cmd_append_it(TAG(20)) ;
    ft801_api_cmd_append_it(VERTEX2F(460*16, 200*16)) ; // down
    ft801_api_cmd_append_it(END()) ;
    
    
    
    // for now draw the up and down button
    
    
    

//    // version with bitmaps    
//    // LOAD text into GPU RAM
//    uint8_t cb = 127 ;
//    uint8_t tab[] = {'s', cb, 'l', cb, 'a', cb, 'w', cb } ;
//    //ft801_api_cmd_memwrite_it(0, (uint8_t*)"slawek pabian :-) :-D", 21) ;
//    
//    ft801_api_cmd_memwrite_it(0,tab, 8);
//    // set the bitmap properties
//    ft801_api_cmd_append_it(CLEAR(1,1,1));
//    
//    
//    ft801_api_cmd_append_it(BITMAP_SOURCE(0));
//    ft801_api_cmd_append_it(BITMAP_LAYOUT(FT_TEXTVGA,7, 3));
//    ft801_api_cmd_append_it(BITMAP_SIZE(FT_BILINEAR,FT_BORDER,FT_BORDER,8*7,272));
//    
//    ft801_api_cmd_append_it(BEGIN(FT_BITMAPS));
//    ft801_api_cmd_append_it(VERTEX2F(0,0));
//    ft801_api_cmd_append_it(END());
    
    
    
    
    ft801_api_cmd_append_it(DISPLAY());
    ft801_api_cmd_append_it(CMD_SWAP);
    
    ft801_api_cmd_flush_it();
    
    
    return true;
}


bool terminalTask_doing(void * const data)
{
    uint16_t * myData = data ;
    
    if ( myData[0] == 1 )
    {
        myData[0] =  0 ; 
        
        uint8_t tag = ft801_spi_rd8(REG_TOUCH_TAG);
        
        if ( tag == 10 ) //up 
        {
            _thisData.m_curr_pos = _get_prev_line() ;
            // refresh task
            ft80x_gpu_eng_it_setActiveTask(TASK_ETERMINAL_ID) ;
        }
        else if ( tag == 20 ) //down
        {
            _thisData.m_curr_pos = _get_next_line() ;
            // refresh task
            ft80x_gpu_eng_it_setActiveTask(TASK_ETERMINAL_ID) ;
        }
        
    } ;
    
    return true;
}


bool terminalTask_gpuit(const uint8_t itflags, void * const data)
{
    uint16_t * myData = data ;

    uint8_t mask = /*FT_INT_CONVCOMPLETE | */ FT_INT_TAG /*| FT_INT_TOUCH*/ ;
    if ( itflags & mask )
    {
        myData[0] = 1 ;
    }
    else
    {
        myData[0] = 0 ;
        
    }
    
    return true;
}




void terminalTask_append_line( char * str )
{
    // compute the needed params to handle text wraping on the screen
    _thisData.m_lines = _thisData.m_area_h / _thisData.m_font_h ;
    _thisData.m_chars = _thisData.m_area_w / _thisData.m_font_w ;
    
    
    size_t b_pos = _thisData.m_curr_pos ;
    for ( size_t c = 0, char_no = 1;
            (str[c] != '\0') && (b_pos < _thisData.m_size-1) ;
            ++c, ++char_no, ++b_pos )
    {
        // conwert to new line if area boundary (x) was reached
        if ( char_no > _thisData.m_chars )
        {
            char_no = 1;
            
            // append \0 to indicate the new line
            _thisData.m_buff[ b_pos ] = '\0' ;
            ++b_pos ;
            
            // check with buffer free space
            if ( b_pos >= (_thisData.m_size -2))
                break ;
        }
        
        if ( '\n' == str[c] )
        {
            //if ( 0 < c ) {
                // replace with '\0'
                char_no = 1 ;
                _thisData.m_buff[ b_pos ] = '\0' ;
                
            //}
        }
        else
        {
            // copy the byte
            _thisData.m_buff[ b_pos ] = str[c] ;
        }

    }
    
    // append the \0
    _thisData.m_buff[ b_pos ] = '\0' ;
    _thisData.m_curr_pos = b_pos + 1 ;
    
}





static size_t _get_prev_line(void)
{
    
    if ( 0 == _thisData.m_curr_pos ) 
        return _thisData.m_curr_pos ;
    
    
    
    int32_t pos = _thisData.m_curr_pos - 2 ; // this always should points to the
                                        // beginning of each line, so substrate of 2 
                                        // is needed for skiping \0 of prev. str.
    
    uint8_t * buff = _thisData.m_buff ;
    for( ; (pos >= 0 ) ; --pos ) {
        if ( buff[pos] == '\0' ) 
            break ;
    }
    
    ++pos ; // points to valid string or to the beggining of the line
    
    return pos ;
}

static size_t _get_next_line(void)
{
    size_t pos = strlen( (char*)&_thisData.m_buff[ _thisData.m_curr_pos] ) + 1;
    
    return _thisData.m_curr_pos + pos ;
}

