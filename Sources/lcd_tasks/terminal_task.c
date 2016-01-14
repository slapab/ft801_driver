#include "terminal_task.h"
#include "FT801_gpu.h"
#include "lcd_tasks_ids.h"

#include "ft80x_it_api_cmd.h"
#include "ft80x_engine_it.h"
#include "spi.h"


#include <stddef.h>

#define SCROLL_TAG 150
#define TEXT_TAG 151
#define TEXT_UP_TAG 152
#define TEXT_DW_TAG 153
#define KEYBOARD_TAG 164


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

static const uint16_t _area_width = 476 ;
static const uint16_t _area_height = 230 ;
static struct _container_t _thisData =
    {
        // AREA USED FOR DISPLAYING STRINGS
        {
            _area_width, // width of area
            _area_height, // height of area
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
static void _recalculate_area(void);
static void _point_to_latest_screen(void) ;
static void _print_text(void);
static bool _is_rb_full(void) ;
static bool _is_rb_empty(void) ;
static void _print_scrollbar(void);
static void _print_text_dw_up(void);
static void _print_keybard_button(void);
static void _print_keybard(void);


static bool show_keyboard = false ;
    

bool terminalTask_painting (void * const data)
{
    uint16_t * myData = data ;
    
    // start creating the screen
    
    ft801_api_cmd_prepare_it(FT_RAM_CMD) ;
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    
    ft801_api_cmd_append_it(CLEAR(1,1,1));
    
    
    // print the text
    _print_text() ;
    
    
    // draw frame rects around the text
    uint8_t lw = 2 ;
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(LINE_WIDTH(2*16) );
    ft801_api_cmd_append_it(COLOR_RGB(0x21,0x96,0xF3) );
    ft801_api_cmd_append_it(COLOR_A(127)) ;
    ft801_api_cmd_append_it(LINE_WIDTH(lw*16) );
    ft801_api_cmd_append_it(BEGIN(FT_LINE_STRIP));
    ft801_api_cmd_append_it(VERTEX2II(lw, 0,0,0));
    ft801_api_cmd_append_it(VERTEX2II(lw, (_thisData.ms_disp_conf.m_area_h),0,0));
    ft801_api_cmd_append_it(VERTEX2II((_thisData.ms_disp_conf.m_area_w+lw), _thisData.ms_disp_conf.m_area_h,0,0));
    ft801_api_cmd_append_it(VERTEX2II((_thisData.ms_disp_conf.m_area_w+lw), 0,0,0));
    
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    
    
    // draw the text up down navigation
    
    _print_text_dw_up() ;

    
    // print the keyboard button
    _print_keybard_button() ;
    
    // print keyboard
    _print_keybard() ;
    
    
    ft801_api_cmd_append_it(TAG(1));
    
    
    
    
    
    ft801_api_cmd_append_it(DISPLAY());
    ft801_api_cmd_append_it(CMD_SWAP);
    ft801_api_cmd_flush_it() ;
    
    return true;
}


bool terminalTask_doing(void * const data)
{
    uint16_t * myData = data ;
    
    if ( myData[0] == 1 )
    {
        myData[0] =  0 ; 
        
        //uint32_t range = ft801_spi_rd32(REG_TRACKER);
        //uint8_t tag = range & (0xFF);//ft801_spi_rd8(REG_TOUCH_TAG);
        uint8_t tag = ft801_spi_rd8(REG_TOUCH_TAG);
        //range >>= 16; //get the range value
        
        if ( tag == TEXT_UP_TAG )
        {
            _move_to_prev_line() ;
            // refresh task
            ft80x_gpu_eng_it_setActiveTask(TASK_ETERMINAL_ID) ;
        }
        else if ( tag == TEXT_DW_TAG ) //down
        {
            _move_to_next_line() ;
            // refresh task
            ft80x_gpu_eng_it_setActiveTask(TASK_ETERMINAL_ID) ;
        }
        else if ( tag == KEYBOARD_TAG )
        {
            
            if ( true == show_keyboard )
            {
                show_keyboard = false ;
                _thisData.ms_disp_conf.m_area_h = _area_height ;
                _thisData.ms_disp_conf.m_area_w = _area_width ;
            }
            else
            {
                show_keyboard = true ;
                _thisData.ms_disp_conf.m_area_h = 100 ;
                _thisData.ms_disp_conf.m_area_w = _area_width ;
                
            }
            _recalculate_area() ;
            _point_to_latest_screen();
            // refresh screen
            ft80x_gpu_eng_it_setActiveTask(TASK_ETERMINAL_ID) ;
        }
        
    }
    
    return true;
}


bool terminalTask_gpuit(const uint8_t itflags, void * const data)
{
    uint16_t * myData = data ;

    uint8_t mask = /*FT_INT_CONVCOMPLETE |*/ FT_INT_TAG | FT_INT_TOUCH ;
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







// This function will print the lines, it will handle the 
// correct lines to display
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
        y_pos += _thisData.ms_disp_conf.m_font_h ;
    }
}






static void _print_text_dw_up(void)
{
    if ( _thisData.m_total_lines_no <= _thisData.m_lines )
    {
        return ;
    }
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_A(30));
    ft801_api_cmd_append_it(BEGIN(FT_RECTS)) ;
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255)) ;
    
    if ( _thisData.m_curr_line_no > 1 )
    {
        ft801_api_cmd_append_it(TAG(TEXT_UP_TAG)) ;
        ft801_api_cmd_append_it(VERTEX2II(0,0,0, 0)) ; // up 
        ft801_api_cmd_append_it(VERTEX2II(_thisData.ms_disp_conf.m_area_w-5, 
            20,0,0)) ; // up 
    }
    
    if ( (_thisData.m_curr_line_no + _thisData.m_lines -1) != _thisData.m_total_lines_no ) 
    {
        ft801_api_cmd_append_it(TAG(TEXT_DW_TAG)) ;
        ft801_api_cmd_append_it(VERTEX2II(0,
            _thisData.ms_disp_conf.m_area_h-20,0, 0)) ; // dw 
        ft801_api_cmd_append_it(VERTEX2II(_thisData.ms_disp_conf.m_area_w-5, 
            _thisData.ms_disp_conf.m_area_h,0,0)) ; // dw
    }
    ft801_api_cmd_append_it(TAG(1)) ;
    ft801_api_cmd_append_it(END()) ;
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
}





static void _print_keybard_button(void)
{
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    
    // print the cancel button
    ft801_api_cmd_fgcolor_it(COLOR_RGB(0xF4,0x43,0x36));//#F44336
    ft801_api_cmd_button_it(360, 240, 50, 28, 18, FT_OPT_FLAT, "X");
    // print the time e.g
    ft801_api_cmd_text_it(440, 248, 18, 0, "18:37");
    
    // print show keyboard button
    ft801_api_cmd_append_it(COLOR_RGB(0xFF,0xFF,0xFF)) ;
    ft801_api_cmd_fgcolor_it(COLOR_RGB(0x21,0x96,0xF3));
    if ( false == show_keyboard) 
    {
        ft801_api_cmd_append_it(TAG(KEYBOARD_TAG));
        ft801_api_cmd_button_it(5, 250, 30, 20, 26, FT_OPT_FLAT, "^") ;
    }
    else
    {
        ft801_api_cmd_append_it(TAG(KEYBOARD_TAG));
        ft801_api_cmd_button_it(5, 250, 30, 20, 18, FT_OPT_FLAT, "_") ;
    }
    ft801_api_cmd_append_it(TAG(1));
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
}



// this local function prints the keyboard
static void _print_keybard(void)
{
    if ( false == show_keyboard )
        return ;
    
    //"1234567890-="
    const char c_1r[] = "qwertyuiop" ;
    const char c_1rsh[] = "QWERTYUIOP";
    const char c_1rn[] = "1234567890" ;
        
    const char c_2r[] = "asdfghjkl;" ;  
    const char c_2rsh[] = "ASDFGHJKL";
    const char c_2rn[] = "!@#$%^&*()" ; 
    
    const char c_3r[] = "zxcvbnm,./" ;
    const char c_3rsh[] = "ZXCVBNM<>?" ;
    const char c_3rn[] = "-=_+{}|\\:\"" ; 
    
    
    const uint8_t key_w = 24 ;
    const uint8_t key_h = 28 ;
    const uint8_t key_x_dist = 50 ;
    const uint16_t key_y_dist = 272 - ( 4 * key_h ) - 10 ;
    
    const uint8_t key_dist = key_h + 2 ;
    
    const char * pc_1r = c_1r ;
    const char * pc_2r = c_2r ;
    const char * pc_3r = c_3r ;
//    const char * pc_1r = c_1rsh ;
//    const char * pc_2r = c_2rsh ;
//    const char * pc_3r = c_3rsh ;
    
//    const char * pc_1r = c_1rn ;
//    const char * pc_2r = c_2rn ;
//    const char * pc_3r = c_3rn ;
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    
    
    ft801_api_cmd_append_it(COLOR_RGB(0x00,0x00,0x00)) ;
    ft801_api_cmd_fgcolor_it(COLOR_RGB(0x9E,0x9E,0x9E));//#9E9E9E
    
    
    ft801_api_cmd_keys_it( key_x_dist, key_y_dist, 12*key_w, key_h, 18, 0, pc_1r) ;
    ft801_api_cmd_keys_it( key_x_dist, key_y_dist+key_dist, 12*key_w, key_h, 18, 0, pc_2r) ;
    ft801_api_cmd_keys_it( key_x_dist, key_y_dist+2*key_dist, 12*key_w, key_h, 18, 0, pc_3r) ;
    // shift key
    ft801_api_cmd_button_it( key_x_dist , key_y_dist+(3*key_dist),
           _thisData.ms_disp_conf.m_font_w+(2*key_w), key_h, 18, 0, "Shift");
    // space key
    ft801_api_cmd_button_it( key_x_dist + 4*key_w, key_y_dist+3*key_dist,
            (4*key_w), key_h, 18,0, "") ; 
    // ?123 key
    ft801_api_cmd_button_it( key_x_dist + (2*_thisData.ms_disp_conf.m_font_w) + (9*key_w),
            key_y_dist+(3*key_dist),
            _thisData.ms_disp_conf.m_font_w + (2*key_w), key_h, 18,0, "!123") ;
    
    // the OK/APPLY button
    ft801_api_cmd_append_it(SAVE_CONTEXT()) ;
    ft801_api_cmd_append_it( COLOR_RGB( 255, 255, 255) ) ;
    ft801_api_cmd_fgcolor_it(COLOR_RGB(0x8B, 0xC3, 0x4A)); //8BC34A
    ft801_api_cmd_button_it( 360, 180, 50, 28, 18, FT_OPT_FLAT, "Apply");
    ft801_api_cmd_append_it(RESTORE_CONTEXT()) ;
    
    // PRINT THE INPUT AREA
    ft801_api_cmd_append_it( COLOR_RGB( 255, 255, 255) ) ;
    ft801_api_cmd_append_it(COLOR_A(70)) ;
    ft801_api_cmd_append_it(BEGIN(FT_RECTS));
    ft801_api_cmd_append_it(VERTEX2II( 5, key_y_dist - 37 , 0, 0 ));
    ft801_api_cmd_append_it(VERTEX2II( 480-5, key_y_dist - 14 , 0, 0 ));
    ft801_api_cmd_append_it(END());
    
    // PRINT THE TEXT IN INPUT AREA
    ft801_api_cmd_append_it(COLOR_A(255)) ;
    ft801_api_cmd_text_it(8, key_y_dist - 33, 18, 0, "") ;
    
    
    
    
    // FINISH
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
}




/**
*   This function copies the given string into local buffer.
*   @param str pointer to the buffer with string to be copied
*/
void terminalTask_appendStr( const char * str )
{
    uint8_t * const pbuff  =  _thisData.ms_rbuff.m_buff ;

    volatile uint32_t * hidx = &_thisData.ms_rbuff.m_head ;
    volatile uint32_t * tidx = &_thisData.ms_rbuff.m_tail ;
    const size_t size = _thisData.ms_rbuff.m_size ;



    // compute the needed params to handle text right edge on the screen
    _recalculate_area();

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
        _point_to_latest_screen();
    }
}










// ########################################
// LOCAL HELPER FUNCTIONS
// ########################################

// this function points to the last - x lines ( when x is calculated 
// acordingly of height of used area)
static void _point_to_latest_screen(void)
{
    const size_t size = _thisData.ms_rbuff.m_size;
    uint8_t * const pbuff = _thisData.ms_rbuff.m_buff ;
         
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





inline static void _recalculate_area(void)
{
    // compute the needed params to handle text right edge on the screen
    _thisData.m_lines = _thisData.ms_disp_conf.m_area_h / _thisData.ms_disp_conf.m_font_h ;
    _thisData.m_chars = _thisData.ms_disp_conf.m_area_w / _thisData.ms_disp_conf.m_font_w ;
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


