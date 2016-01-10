#include "terminal_task.h"
#include "FT801_gpu.h"
#include "lcd_tasks_ids.h"

#include "ft80x_it_api_cmd.h"
#include "ft80x_engine_it.h"
#include "spi.h"


#include <stddef.h>



struct _ringbuffer_t
{
    uint8_t * m_buff ;
    size_t m_size ;

    volatile uint32_t m_head ; // index
    volatile uint32_t m_tail ; // index


};

struct _disparea_conf_t
{
    /// define the area used to display text
    uint16_t m_area_w ;
    uint16_t m_area_h ;

    /// sizes of used font | refer to the datasheet of the chip
    uint8_t m_font_w ;
    uint8_t m_font_h ;
};


// internal buffer for storing lines
struct _container_t
{
    /// keeps the configuration of display area and font
    struct _disparea_conf_t ms_disp_conf ;

    /* ALL which consist of text buffer */
    /// buffer for storing the displayed text
    struct _ringbuffer_t ms_rbuff ;
    /// the line from which it will be start printing
    size_t m_start_line ; // index

    /// keeps total number of strings in the buffer
    size_t m_total_lines_no ;

    /// keeps number of current line from the buffer
    size_t m_curr_line_no ;


    /* calculated by software */

    /// chars in line
    uint8_t m_chars ;
    /// total line in given area
    uint8_t m_lines ;

} ;



// Define the internal ring buffer for storing characters
static const uint16_t _rbuffer_size = 2048 ; // NOTE this value should be power of 2!
static uint8_t _string_buffer[_rbuffer_size] ;

static struct _container_t _thisData =
    {
        // AREA USED FOR DISPLAYING STRINGS
        {
            435, // width of area
            100, // height of area
            8,   // width of used font ( ft801: 18/19 )
            16   // height of used font ( ft801: 18/19 )
        },

        // RING BUFFER FOR STRING STRINGS
        {
            _string_buffer, // pointer to the buffer
            _rbuffer_size,  // size of the buffer
            0,              // head to zero
            0,              // tail to zero
        },

        // DATA USED FOR HANDLING THE DISPLAYED DATA
        0,  // the start line (string) in the buffer
        0,  // total lines in the ring buffer
        0,  // number of current line pointed by m_start_line


        // VARIABLES WHICH DEFINE THE MAXIMUM CHARACTERS INSIDE OF AREA
        // THIS VALUES ARE CALCULATED BY SOFTWARE
        0,
        0
    };




// forward declaration of local functions: 
static void _move_to_next_line(void) ;
static void _move_to_prev_line(void) ;
static void _print_text(void);
static bool _is_rb_full(void) ;
static bool _is_rb_empty(void) ;




    
    

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
    
    
    // print the text
    _print_text() ;
    
    // draw frame rects
    uint8_t d = 2 * _thisData.m_lines ;
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(LINE_WIDTH(2*16) );
    ft801_api_cmd_append_it(COLOR_RGB(0, 80, 100) );
    ft801_api_cmd_append_it(LINE_WIDTH(2*16) );
    ft801_api_cmd_append_it(BEGIN(FT_LINE_STRIP));
    ft801_api_cmd_append_it(VERTEX2F(0, 0));
    ft801_api_cmd_append_it(VERTEX2F(0, 16*(_thisData.ms_disp_conf.m_area_h+d)));
    ft801_api_cmd_append_it(VERTEX2F(16*(_thisData.ms_disp_conf.m_area_w+d), 16*(_thisData.ms_disp_conf.m_area_h+d)));
    ft801_api_cmd_append_it(VERTEX2F(16*(_thisData.ms_disp_conf.m_area_w+d), 0));
    
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    
    
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
            _move_to_prev_line() ;
            // refresh task
            ft80x_gpu_eng_it_setActiveTask(TASK_ETERMINAL_ID) ;
        }
        else if ( tag == 20 ) //down
        {
            _move_to_next_line() ;
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



void _print_text(void)
{
    // handle the text display:
    int16_t x_pos = 5 ;
    int16_t y_pos = 0 ;
    
    if ( true == _is_rb_empty() )
    {
        return ;
    }

    uint8_t * const pbuff = _thisData.ms_rbuff.m_buff ;
    const size_t size = _thisData.ms_rbuff.m_size ;

    uint32_t start_line = _thisData.m_start_line ;

    int32_t l_no = (_thisData.m_lines > _thisData.m_total_lines_no)?
            (_thisData.m_total_lines_no) : (_thisData.m_lines) ;

    char tmp_buff [ 480/8 ] ;
    // display lines
    for( ;0 < l_no; --l_no )
    {
        uint8_t i = 0;
        while( pbuff[start_line] != '\0' ) //&&  (hidx != start_line) )
        {
            tmp_buff[i] = (char)pbuff[start_line] ;
            ++i ;
            ++start_line ;
            
            // wrap the ring buffer
            if ( size <= start_line)
                start_line = 0 ;
        }
        tmp_buff[i] = '\0' ;
        ++start_line ; // skip the null and point to first char of the next line
        if ( size <= start_line )
        {
            start_line = 0;
        }
        
        // push to the LCD buffer
        ft801_api_cmd_text_it(x_pos, y_pos, 18, 0, tmp_buff);
        y_pos += _thisData.ms_disp_conf.m_font_h + 2;
    }
}







void terminalTask_appendStr( const char * str )
{
    uint8_t * const pbuff  =  _thisData.ms_rbuff.m_buff ;

    volatile uint32_t * hidx = &_thisData.ms_rbuff.m_head ;
    volatile uint32_t * tidx = &_thisData.ms_rbuff.m_tail ;
    const size_t size = _thisData.ms_rbuff.m_size ;



    // compute the needed params to handle text right edge on the screen
    _thisData.m_lines = _thisData.ms_disp_conf.m_area_h / _thisData.ms_disp_conf.m_font_h ;
    _thisData.m_chars = _thisData.ms_disp_conf.m_area_w / _thisData.ms_disp_conf.m_font_w ;


    // copy string to the internal ring buffer
    for ( size_t c = 0, char_no = 1 ; str[c] != '\0' ; ++c, ++char_no )
    {
        if ( true == _is_rb_full() )
        {
            // CHECK IF ATTEMPING COPY CHAR. WILL OVERWRITE THE OLD STRING
            if ( ('\0' == pbuff[ *hidx ]) && ( 0 < _thisData.m_total_lines_no) )
            {
               -- _thisData.m_total_lines_no ;
            }

           // Move tail forward by one : calculate value by use optimalized modulo operation
           *tidx = ((*tidx)+1) & ( size-1 ) ;
        }



        // HANDLE LINE OVERFLOW ( create new string ):
        if ( char_no > _thisData.m_chars )
        {
            char_no = 1;
            // append \0 to indicate the new line
            pbuff[ *hidx ] = '\0' ;
            *hidx = ( (*hidx) +1) & ( size - 1 ) ;

            // update buffer statistics
            ++ _thisData.m_total_lines_no;

            // check fullness again, if is full then move forward the tail
            if ( true == _is_rb_full() )
            {
               *tidx = ((*tidx) +1) & ( size-1 ) ;
            }
        }

        //APPEND CHARACTER TO THE RING BUFFER
        if ( '\n' == str[c] )
        // Handle \n character -> push it as \0
        {
            char_no = 1 ;
            pbuff[ *hidx ] = '\0' ;

            // update buffer statistics
            ++ _thisData.m_total_lines_no;
        }
        else
        // push normal character
        {
            pbuff[ *hidx ] = str[c] ;
        }

        // calculate the new head value by use optimalized modulo operation
        *hidx = ( (*hidx) +1 ) & ( size - 1 ) ;
    }

    // add the end of string character for the new string [ the last string ]
    if ( true == _is_rb_full() )
    {
       *tidx = ((*tidx)+1) & ( size-1 ) ;
    }

    pbuff[ *hidx ] = '\0' ;
    *hidx = ( (*hidx) + 1) & ( size - 1 ) ;
    // Update buffer statistics
    ++ _thisData.m_total_lines_no ;


    // DELETE BORKEN STRING IF OVERRFLOW OCCURED
    if ( true == _is_rb_full() )
    {
       *tidx = *hidx ;//((*tidx)+1) & ( size-1 ) ;

       // now 'delete' the old broken string, and move tail to the first
       // following, good one string
       bool test = false ;
       while ( pbuff[*tidx] != '\0' )
       {
           *tidx = ((*tidx)+1) & ( size-1 ) ;
           test = true ;
       }
       // now move tail pointer to the first element of good one string
       *tidx = ((*tidx)+1) & ( size-1 ) ;

       // Update buffer statistics
       if ( (true == test) && ( 1 < _thisData.m_total_lines_no ) )
           -- _thisData.m_total_lines_no ;
    }


    // UPDATE THE m_start_line INDEX!
    if ( _thisData.m_total_lines_no <= _thisData.m_lines )
    {
        _thisData.m_start_line = _thisData.ms_rbuff.m_tail ;
        _thisData.m_curr_line_no = 1;
    }
    else
    // SET THE START LINE AT m_lines from the end of buffer
    {
        uint32_t pos = ( _thisData.ms_rbuff.m_head - 2 ) & (size -1); // -2 to skip \0 of the first line
        for ( int32_t i = _thisData.m_lines ;
                0 < i ; --i)
        {
            while ( pbuff[pos] != '\0')
            {
                pos = (pos -1 ) & ( size - 1);
            }
            pos = (pos -1 ) & ( size - 1);

        }

        _thisData.m_start_line = (pos +2 ) & ( size - 1) ;
        _thisData.m_curr_line_no = _thisData.m_total_lines_no - _thisData.m_lines +1 ;
    }
}










// LOCAL HELPER FUNCTIONS

static void _move_to_prev_line(void)
{

    const uint32_t tidx = _thisData.ms_rbuff.m_tail ;
    const size_t size = _thisData.ms_rbuff.m_size ;

    if ( true == _is_rb_empty() )
    {
        _thisData.m_start_line = _thisData.ms_rbuff.m_head ;
        return;
    }
    else if ( tidx == _thisData.m_start_line )
    // do not go back not more than tail
    {
        return ;
    }


    // this always should points to the
    // beginning of each line, so substrate of 2
    // is needed for skipping \0 of prev. str.
    uint32_t pos = _thisData.m_start_line - 2 ;

    uint8_t * buff = _thisData.ms_rbuff.m_buff ;
    for( ; (tidx != pos ) ;  pos = (pos - 1) & (size -1 ) ) {
        if ( buff[pos] == '\0' )
            break ;
    }
    // update statistics
    --_thisData.m_curr_line_no ;

    if ( tidx == pos )
        _thisData.m_start_line = pos ;
    else
        // points to valid string or to the beginning of the line
        _thisData.m_start_line = (pos + 1) & (size -1) ;
}







static void _move_to_next_line(void)
{
    // do not go further if all remaining lines can be displayed on the area:
    if ( true == _is_rb_empty() )
    // nothing to display, set the start_line to the tail(head) index
    {
        _thisData.m_start_line = _thisData.ms_rbuff.m_tail ;
        return ;
    }
    else if ( _thisData.m_total_lines_no <= _thisData.m_lines )
    // all lines can be displayed on the one screen, so set start line to the tail
    {
        _thisData.m_start_line = _thisData.ms_rbuff.m_tail ;
        return ;
    }

    // more than one screen can be displayed

    const uint32_t left_lines = _thisData.m_total_lines_no - _thisData.m_curr_line_no +1 ;

    if ( left_lines > _thisData.m_lines )
    // ok, can go forward by one line
    {
        const uint8_t  * const pbuff = _thisData.ms_rbuff.m_buff;
        const size_t size = _thisData.ms_rbuff.m_size ;
        const uint32_t hidx = _thisData.ms_rbuff.m_head ;
        uint32_t idx = _thisData.m_start_line ;

        // go to the next line
        for ( ; idx != hidx ; idx = (idx+1) & ( size -1))
        {
            if (pbuff[idx] == '\0')
            // found the end of the current line
            {
                // update, point to the first char of the next line
                _thisData.m_start_line = (idx+1) & ( size -1 ) ;
                // update statistics
                ++_thisData.m_curr_line_no ;
                break ;
            }
        }
    }
    return ;
}



inline static bool _is_rb_full(void)
{
    const uint32_t hidx = (_thisData.ms_rbuff.m_head + 1) & (_thisData.ms_rbuff.m_size - 1) ;
    if ( hidx == _thisData.ms_rbuff.m_tail )
        return true ;
    else
        return false ;
}

inline static bool _is_rb_empty(void)
{
    if ( _thisData.ms_rbuff.m_head == _thisData.ms_rbuff.m_tail )
        return true ;
    else
        return false ;
}


