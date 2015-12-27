#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "ft80x_task.h"

#include "ft80x_it_api_cmd.h"
#include "ft80x_engine_it.h"

#include "my_tasks.h"

#include "spi.h" // remove this dependecy - must deal with it

#include "ft801_gpu.h"

bool task1_painting(void * const data)
{
    ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
    
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(50, 50, 55));
    ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
    ft801_api_cmd_append_it(COLOR_RGB(0,100,250));
    ft801_api_cmd_append_it( TAG(2) );
    ft801_api_cmd_text_it(240,136,30,FT_OPT_CENTER,"And what now?") ;
    ft801_api_cmd_append_it( TAG(3) );
    ft801_api_cmd_text_it(240,166,30,FT_OPT_CENTER,"Keyboard task") ;
    ft801_api_cmd_append_it(TAG_MASK(0)) ;

    ft801_api_cmd_append_it(DISPLAY()) ;
    ft801_api_cmd_append_it(CMD_SWAP);

    ft801_api_cmd_flush_it();
    
    return true ;
}

bool task1_doing( void * const data )
{
    uint8_t * myData = data ;
    
    if ( 2 == myData[0] )
    {
        myData[0] = 0;
        
        // read touch tag register
        uint8_t tag = ft801_spi_rd8(REG_TOUCH_TAG);
        
        if ( 2 == tag )
        {
            // set new Active task
            ft80x_gpu_eng_it_setActiveTask(TASK_ID2) ;
        }
        else if ( 3 == tag )
        {
             // set the active task to keyboard task
            ft80x_gpu_eng_it_setActiveTask(TASK_KEYBOARD) ;
        }
    }
    
    return true;
}


bool task1_gpuit( const uint8_t itflags, void * const data )
{
    uint8_t * myData = data ;
    if ( itflags & FT_INT_TAG  )
    {
        *myData = 2 ;
    }
    else
    {
        *myData = 0 ;
    }
    return true;
}






// ### TASK 2
const size_t str_task2_size = 8 ;
char str_task2[str_task2_size] = {0,0,0,0,0,0,0, 0};

bool task2_painting(void * const data)
{
    uint16_t * myData = data ;
    
    ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
    
    
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(0, 120, 255));
    ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    ft801_api_cmd_append_it( TAG(3) );
    ft801_api_cmd_text_it(240,50,30,FT_OPT_CENTER,"Back to HOME") ;
    
    ft801_api_cmd_append_it(TAG(4) );
    ft801_api_cmd_slider_it(120,100, 240,10, FT_OPT_FLAT, myData[2], 65535) ;
    ft801_api_cmd_track_it(120, 90, 240, 20, 4);
    //ft801_api_cmd_append_it(TAG(100) );
    ft801_api_cmd_append_it(TAG_MASK(0)) ;
    
    ft801_api_cmd_text_it(120, 130, 28, 0, str_task2 ) ;
    ft801_api_cmd_keys_it( 120, 170, 4*32,35, 28, myData[1], "ABCD");
    
    
    //ft801_api_cmd_append_it(TAG(5) );
    
    ft801_api_cmd_append_it(DISPLAY()) ;
    ft801_api_cmd_append_it(CMD_SWAP);
    
    
    
    ft801_api_cmd_flush_it();
    
    return true ;
}

bool task2_doing( void * const data )
{
    static size_t loc_str_idx = 0 ;
    
    uint16_t * myData = data ;
    
    if ( myData[0] == 1 )
    {
        myData[0] = 0 ;
        
        uint32_t tmp = ft801_spi_rd32(REG_TRACKER);
        //uint8_t tag = (uint8_t)(tmp >> 16) ;
        
        // read touch tag register
        //uint8_t tag = ft801_spi_rd8(REG_TOUCH_TAG);
        uint8_t tag = tmp & 0xFF ;
        if ( tag == 3 )
        {
            // set new Active task
            ft80x_gpu_eng_it_setActiveTask(TASK_ID1) ;
        }
        else if( tag == 4 )
        {
            // read progress
            uint32_t tmp = ft801_spi_rd32(REG_TRACKER);
            tmp >>= 16 ; // catch only change
            // repaint this screen with new change value
            myData[2] = tmp ;
            // set new Active task
            ft80x_gpu_eng_it_setActiveTask(TASK_ID2) ;
        }
        else if ( (tag == 'A') || (tag == 'B') || (tag == 'C') || (tag == 'D')  )
        {
            myData[1] = tag ;
            // update str value
            str_task2[loc_str_idx] = tag ;
            loc_str_idx = (loc_str_idx+1) & ( str_task2_size -1 ) ; // must be power of 2
            if ( loc_str_idx == 7 )
            {   
                str_task2[loc_str_idx] = 0 ;
                loc_str_idx = 0;
            }
            // set new Active task
            ft80x_gpu_eng_it_setActiveTask(TASK_ID2) ;
        }
        else
        {
            myData[1] = 0;
            // set new Active task
            ft80x_gpu_eng_it_setActiveTask(TASK_ID2) ;
        }
    }
    
    return true;
}



bool task2_gpuit( const uint8_t itflags, void * const data )
{
    uint16_t * myData = data ;
    uint8_t mask = /*FT_INT_CONVCOMPLETE | */ FT_INT_TAG | FT_INT_TOUCH ;
    if ( itflags & mask )
    {
        myData[0] = 1 ;
        myData[1] = 0 ;
    }
    else
    {
        myData[0] = 0 ;
        
    }
    
    return true;
}













// keyboard task
static uint16_t keys_pressed = 0;
bool keyboardTask_painting( void * const data )
{
 
     uint16_t * myData = data ;
    
    ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
    
    
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(0, 120, 255));
    ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
//    ft801_api_cmd_append_it( TAG(3) );
//    ft801_api_cmd_text_it(240,50,30,FT_OPT_CENTER,"Back to HOME") ;
    
//    ft801_api_cmd_append_it(TAG(4) );
//    ft801_api_cmd_slider_it(120,100, 240,10, FT_OPT_FLAT, myData[2], 65535) ;
//    ft801_api_cmd_track_it(120, 90, 240, 20, 4);
//    ft801_api_cmd_append_it(TAG(100) );
    //ft801_api_cmd_append_it(TAG_MASK(0)) ;
    
    // draw the rectangle for text inuput
    ft801_api_cmd_append_it( LINE_WIDTH(2*16) );
    ft801_api_cmd_append_it( BEGIN(FT_RECTS) );
    ft801_api_cmd_append_it( VERTEX2F( 10 * 16, 10 * 16) );
    ft801_api_cmd_append_it( VERTEX2F( 470 * 16, 80 * 16 ) );
    ft801_api_cmd_append_it( END() );
    
    // draw the keyboard
    
    // the alpha keys
    ft801_api_cmd_keys_it( 10, 100, 10*32,32, 26, keys_pressed, "qwertyuiop");
    ft801_api_cmd_keys_it( 10, 135, 9*32,32, 26, keys_pressed, "asdfghjkl");
    ft801_api_cmd_keys_it( 10, 170, 7*32,32, 26, keys_pressed, "zxcvbnm");
    ft801_api_cmd_button_it( 10 + (7*32) + 5, 170, 90, 32, 26, keys_pressed, "Backspace" );
    ft801_api_cmd_button_it( 10, 205, 60, 32, 26, keys_pressed, "Shift" );
    ft801_api_cmd_keys_it( 75, 205, 3*32,32, 26, keys_pressed, ", .");
    // the numeric keys
    ft801_api_cmd_keys_it( (10*32) + 40, 100, 3*32,32, 26, keys_pressed, "789");
    ft801_api_cmd_keys_it( (10*32) + 40, 135, 3*32,32, 26, keys_pressed, "456");
    ft801_api_cmd_keys_it( (10*32) + 40, 170, 3*32,32, 26, keys_pressed, "123");
    ft801_api_cmd_keys_it( (10*32) + 40, 205, 3*32,32, 26, keys_pressed, "+0-");
    
    
    
    //ft801_api_cmd_text_it(120, 130, 28, 0, str_task2 ) ;
    
    
    
    //ft801_api_cmd_append_it(TAG(5) );
    
    ft801_api_cmd_append_it(DISPLAY()) ;
    ft801_api_cmd_append_it(CMD_SWAP);
    
    
    
    ft801_api_cmd_flush_it();

    
    return true ;
}



// keyboard task
bool keyboardTask_doing( void * const data )
{
    uint16_t * myData = data ;
    if ( myData[0] == 1 )
    {
        myData[0] = 0 ;
        
        
        uint8_t tag = ft801_spi_rd8(REG_TOUCH_TAG);
        
        // handle the keys touch
        if ( (tag > 0x20) && ( tag < 0x7E) )
        {
            keys_pressed = tag ;
            // update changes on the screen
            ft80x_gpu_eng_it_setActiveTask(TASK_KEYBOARD) ;
        }
        else
        {
            keys_pressed = 0 ;
            // update changes on the screen
            ft80x_gpu_eng_it_setActiveTask(TASK_KEYBOARD) ;
        }
    }
    
    
    return true ;
}




// keyboard task
bool keyboardTask_gpuit( const uint8_t itflags, void * const data )
{
    uint16_t * myData = data ;
    
    uint8_t mask = /*FT_INT_CONVCOMPLETE | */ FT_INT_TAG | FT_INT_TOUCH ;
    if ( itflags & mask )
    {
        myData[0] = 1 ;
    }
    else
    {
        myData[0] = 0 ;
    }
    
    return true ;
}



