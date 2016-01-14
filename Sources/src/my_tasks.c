#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "ft80x_task.h"

#include "ft80x_it_api_cmd.h"
#include "ft80x_engine_it.h"

#include "my_tasks.h"

#include "spi.h" // remove this dependecy

#include "ft801_gpu.h"

bool task1_painting(void * const data)
{
    ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
    
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(50, 50, 55));
    ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
    ft801_api_cmd_append_it(COLOR_RGB(0,100,250));
    ft801_api_cmd_append_it( TAG(2) );
    ft801_api_cmd_text_it(240,126,30,FT_OPT_CENTER,"Sliders task") ;
    ft801_api_cmd_append_it( TAG(3) );
    ft801_api_cmd_text_it(240,166,30,FT_OPT_CENTER,"Keyboard task") ;
    ft801_api_cmd_append_it( TAG(4) );
    ft801_api_cmd_text_it(240,206,30,FT_OPT_CENTER,"Graph task") ;
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
        else if ( 4 == tag )
        {
            ft80x_gpu_eng_it_setActiveTask(TASK_GRAPH) ;
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
    
    uint8_t red_color = (uint32_t)(myData[1]*255)/65536 ;
    uint8_t green_color = (uint32_t)(myData[2]*255)/65536 ;
    uint8_t blue_color = (uint32_t)(myData[3]*255)/65536 ;
    
    
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(red_color, green_color, blue_color));
    ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    ft801_api_cmd_append_it( TAG(3) );
    ft801_api_cmd_text_it(240,20,30,FT_OPT_CENTER,"Back to Home") ;
    
    
    
    ft801_api_cmd_append_it( SAVE_CONTEXT()) ;
    
    
    // print the red slider
    ft801_api_cmd_fgcolor_it( COLOR_RGB(240,0,0 ) ) ;
    ft801_api_cmd_append_it(COLOR_RGB(128,0,20));
    ft801_api_cmd_append_it(TAG(200) );
    ft801_api_cmd_slider_it(120,70, 240,10, FT_OPT_FLAT, myData[1], 65535) ;
    ft801_api_cmd_track_it(120, 70, 240, 10, 200);
    
    // print the green slider
    ft801_api_cmd_fgcolor_it( COLOR_RGB(0,240,0 ) ) ;
    ft801_api_cmd_append_it(COLOR_RGB(0,128,20));
    ft801_api_cmd_append_it(TAG(201) );
    ft801_api_cmd_slider_it(120, 110, 240,10, FT_OPT_FLAT, myData[2], 65535) ;
    ft801_api_cmd_track_it(120, 110, 240, 10, 201);
    
    // print the blue slider
    ft801_api_cmd_fgcolor_it( COLOR_RGB(0,0,240 ) ) ;
    ft801_api_cmd_append_it(COLOR_RGB(0,20,128));
    ft801_api_cmd_append_it(TAG(202) );
    ft801_api_cmd_slider_it(120, 150, 240,10, FT_OPT_FLAT, myData[3], 65535) ;
    ft801_api_cmd_track_it(120, 150, 240, 10, 202);
    
    ft801_api_cmd_append_it( RESTORE_CONTEXT()) ;
   
    // display gauge for pwm control
    ft801_api_cmd_append_it(TAG(203)) ;   
    ft801_api_cmd_gauge_it(240, 220, 49, FT_OPT_NOBACK, 5, 3, myData[4], 128 ) ;
    ft801_api_cmd_track_it(240, 220, 1, 1, 203);
    // update the pwm output
    ft801_api_cmd_memwrite_it( REG_PWM_DUTY, (uint8_t*)&myData[4], 1) ;
   
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
        if ( 3 == tag )
        {
            // back to HOME
            ft80x_gpu_eng_it_setActiveTask(TASK_ID1) ;
        }
        // handle red slider
        else if( 200 == tag )
        {
            myData[1] = tmp >> 16 ;
            // set new Active task
            ft80x_gpu_eng_it_setActiveTask(TASK_ID2) ;
        }
        // handle the green slider
        else if ( 201 == tag )
        {        
            myData[2] = tmp >> 16;
            // set new Active task
            ft80x_gpu_eng_it_setActiveTask(TASK_ID2) ;
        }
        // handle the blue slider
        else if ( 202 == tag )
        {        
            myData[3] = tmp >> 16;
            // set new Active task
            ft80x_gpu_eng_it_setActiveTask(TASK_ID2) ;
        }
        // handle the guage
        else if ( 203 == tag )
        {
            myData[4] = tmp >> 16 ;
            myData[4] = (uint32_t)(myData[4]*128)/65536 ;
            
            // refresh screen
            ft80x_gpu_eng_it_setActiveTask(TASK_ID2) ;
        }
        

    }
    
    return true;
}



bool task2_gpuit( const uint8_t itflags, void * const data )
{
    uint16_t * myData = data ;
    uint8_t mask = FT_INT_CONVCOMPLETE | FT_INT_TAG /*FT_INT_TOUCH*/ ;
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













// keyboard task 

// Handle buttons information
static uint16_t keys_pressed = 0;
static uint16_t shift_bt_pressed = 0;
static uint16_t backsapce_bt_pressed = 0 ;
static uint16_t space_bt_pressed = 0 ;
static uint16_t ok_bt_pressed = 0;
static uint16_t cancel_bt_pressed = 0;

const uint8_t str_size = 40 ;
uint8_t str_curr_pos = 0 ;
char str_buff[ str_size ] ;

bool keyboardTask_painting( void * const data )
{
 
    uint16_t * myData = data ;
    
    ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
    
    
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(0, 120, 255));
    ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    
    // draw the rectangle for text inuput
    ft801_api_cmd_append_it( LINE_WIDTH(2*16) );
    ft801_api_cmd_append_it( BEGIN(FT_RECTS) );
    ft801_api_cmd_append_it( VERTEX2F( 10 * 16, 10 * 16) );
    ft801_api_cmd_append_it( VERTEX2F( 470 * 16, 80 * 16 ) );
    ft801_api_cmd_append_it( END() );

    // draw the text inside of rectangle
    ft801_api_cmd_append_it( SAVE_CONTEXT()) ;
    ft801_api_cmd_append_it(COLOR_RGB(0,0,0));
    
    str_buff[ str_curr_pos ] = '|' ;
    str_buff[ str_curr_pos + 1] = '\0' ;
    ft801_api_cmd_text_it( 15, 15, 29, 0, str_buff );
    ft801_api_cmd_number_it( 440, 65, 26, 0, (int32_t)(str_size - str_curr_pos - 2) );
    
    
    ft801_api_cmd_append_it( RESTORE_CONTEXT()) ;



    // draw the keyboard
    
    // set color for keys
    ft801_api_cmd_fgcolor_it(COLOR_RGB(0, 100, 180)) ;
    
    // the keys
    ft801_api_cmd_keys_it( 10, 100, 10*32,32, 26, keys_pressed,
        (shift_bt_pressed != FT_OPT_FLAT)? "qwertyuiop" : "QWERTYUIOP"
    );
    ft801_api_cmd_keys_it( 10, 135, 9*32,32, 26, keys_pressed,
        (shift_bt_pressed != FT_OPT_FLAT)? "asdfghjkl" : "ASDFGHJKL"
    );
    ft801_api_cmd_keys_it( 10, 170, 7*32,32, 26, keys_pressed,
        (shift_bt_pressed != FT_OPT_FLAT)? "zxcvbnm" : "ZXCVBNM"
    );
    ft801_api_cmd_append_it( TAG(200) );
    ft801_api_cmd_button_it( 10 + (7*32) + 5, 170, 90, 32, 26, backsapce_bt_pressed, "Backspace" );
    ft801_api_cmd_append_it( TAG(201) );
    ft801_api_cmd_button_it( 10, 205, 61, 32, 26, shift_bt_pressed, "Shift" );
    ft801_api_cmd_keys_it( 10+64, 205, 2*32,32, 26, keys_pressed, ",.");
    ft801_api_cmd_append_it( TAG(202) );
    ft801_api_cmd_button_it( 10 + 131, 205, 60, 32, 26, space_bt_pressed, "" );
    ft801_api_cmd_append_it( TAG_MASK(0) );
    
    
    
    // the numeric keys
    ft801_api_cmd_keys_it( (10*32) + 40, 100, 3*32,32, 26, keys_pressed,
        (shift_bt_pressed != FT_OPT_FLAT) ? "789" : "!@#"
    );
    ft801_api_cmd_keys_it( (10*32) + 40, 135, 3*32,32, 26, keys_pressed,
        (shift_bt_pressed != FT_OPT_FLAT) ? "456" : "$%^"
    );
    ft801_api_cmd_keys_it( (10*32) + 40, 170, 3*32,32, 26, keys_pressed,
        (shift_bt_pressed != FT_OPT_FLAT) ? "123" : "&*("
    );
    ft801_api_cmd_keys_it( (10*32) + 40, 205, 3*32,32, 26, keys_pressed,
        (shift_bt_pressed != FT_OPT_FLAT) ? "+0-" : ")_;"
    );
    
    
    // save currrent context
    ft801_api_cmd_append_it( SAVE_CONTEXT()) ;
    
    ft801_api_cmd_append_it( TAG_MASK(1) );
    // the OK and cancel button
    ft801_api_cmd_append_it( TAG(203) );
    ft801_api_cmd_fgcolor_it(COLOR_RGB(250, 0, 0)) ;
    ft801_api_cmd_button_it( 220 , 272-34, 70, 28, 26, FT_OPT_FLAT, "CANCEL" ) ;
    
    ft801_api_cmd_append_it( TAG(204) );
    ft801_api_cmd_fgcolor_it(COLOR_RGB(0, 180, 0)) ;
    ft801_api_cmd_button_it( 300 , 272-34, 40, 28, 26, FT_OPT_FLAT, "OK" ) ;
    ft801_api_cmd_append_it( TAG_MASK(0) );
    
    // restore context
    ft801_api_cmd_append_it( RESTORE_CONTEXT()) ;
    
    
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
        
        keys_pressed = 0 ;
        backsapce_bt_pressed = 0 ;  //3D
        space_bt_pressed = 0 ;      //3D
        cancel_bt_pressed = 0 ;
        ok_bt_pressed = 0 ;
        
        
        // handle the keys touch
        if ( ((tag > 0x20) && ( tag < 0x7E)))
        {
            if ( str_curr_pos < ( str_size - 2 ) )
            {
                str_buff[ str_curr_pos ] = tag ;
                ++str_curr_pos ;
            }
                
            keys_pressed = tag ;
        }
        // handle the backspace button pressed
        else if ( 200 == tag )
        {
            if ( str_curr_pos > 0 )
            {
                --str_curr_pos ;
            }
            
            backsapce_bt_pressed = FT_OPT_FLAT ; // flat
        }
        // handle the shift button
        else if ( 201 == tag )
        {
            if ( shift_bt_pressed != 0 )
                shift_bt_pressed = 0 ;
            else
                shift_bt_pressed = FT_OPT_FLAT ;
        }
        // handle the space button
        else if ( 202 == tag )
        {
            if ( str_curr_pos < ( str_size - 2 ) )
            {
                str_buff[ str_curr_pos ] = ' ' ;
                ++str_curr_pos ;
            }
            space_bt_pressed = FT_OPT_FLAT ;
        }
        // handle cancel button
        else if ( 203 == tag )
        {
            // back to previous task
            ft80x_gpu_eng_it_setActiveTask(TASK_ID1) ;
            return true ;
        }
        // handle ok button
        else if ( 204 == tag )
        {
            // store read text
            // back to previous task 
            ft80x_gpu_eng_it_setActiveTask(TASK_ID1) ;
            return true ;
        }
        // update changes on the screen
        ft80x_gpu_eng_it_setActiveTask(TASK_KEYBOARD) ;
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








// THE GRAPH TASK
#define CONV_DIST(x) ((x)*16)
static const uint8_t point_num = 35 ;
static uint16_t x[point_num] = {0, 9, 18, 27, 36, 44, 54, 63, 72, 81, 89, 99, 108, 117, 126, 135, 144, 153, 162, 171, 179, 189, 198, 206, 216, 225, 234, 242, 252, 261, 270, 279, 288, 297, 306};    
static uint16_t y[point_num] = {0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1};
    
static uint16_t x_scale = 0 ;
static uint16_t y_scale = 0 ;
static int16_t tx1 = 0 ;
static int16_t ty1 = 0 ;
static int16_t tx2 = 0 ;
static int16_t ty2 = 0 ;

bool graphExample_painting( void * const data )
{
    uint16_t * myData = data ;
    
    ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
    
    
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(0, 120, 255));
    ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    ft801_api_cmd_append_it( TAG(200) ) ;
    ft801_api_cmd_text_it(10,10, 26, 0, "Back to the home" ) ;
    
    
    
    // print the frame for the graph
    
    uint16_t x_dist = 10 * 16 ; // distance from the left edge
    uint16_t x_width = 460 * 16 ; // width (x) of the rectangle
    uint16_t y_dist = 50 * 16 ; // distance from the top
    uint16_t y_width = 212 * 16 ; // width (y) of the rectangle
    
    ft801_api_cmd_append_it( COLOR_RGB(0, 0, 0) );
    ft801_api_cmd_append_it( TAG(201) ) ;
    ft801_api_cmd_append_it( BEGIN(FT_RECTS) ) ;
    ft801_api_cmd_append_it( LINE_WIDTH(2*16)) ;
    ft801_api_cmd_append_it( VERTEX2F( x_dist, y_dist ) );
    ft801_api_cmd_append_it( VERTEX2F( x_dist + x_width, y_dist+y_width ) );
    ft801_api_cmd_append_it( END() );
    
    // track touching this rect
    ft801_api_cmd_track_it(x_dist, y_dist, x_width, y_width, 201 ) ;
    
    ft801_api_cmd_append_it( TAG_MASK(0) ) ;
    
    
    // draw the plot
    ft801_api_cmd_append_it( COLOR_RGB(200, 200, 200) );
    ft801_api_cmd_append_it( BEGIN(FT_LINES) ) ;
    ft801_api_cmd_append_it( LINE_WIDTH(1*16)) ;
    
    
    uint16_t y_middle = (y_width / 2) + y_dist ;
    for ( uint8_t i = 0 ; i < point_num-1 ; ++i )
    {
        
        // start 
       
        if ( y[i] == 1 )
            ft801_api_cmd_append_it( VERTEX2F( x_dist + CONV_DIST(x[i]) , y_middle - CONV_DIST(y[i]+50) - y_scale) );
        else
            ft801_api_cmd_append_it( VERTEX2F( x_dist + CONV_DIST(x[i]), y_middle + CONV_DIST(y[i]+50) + y_scale  ) );
        
        // end
        
        if ( y[i+1] == 1 )
            ft801_api_cmd_append_it( VERTEX2F( x_dist + CONV_DIST(x[i+1]), y_middle - CONV_DIST(y[i+1]+50) - y_scale ) );
        else
            ft801_api_cmd_append_it( VERTEX2F( x_dist + CONV_DIST(x[i+1]), y_middle + CONV_DIST(y[i+1]+50) + y_scale ) );
        
    }
    ft801_api_cmd_append_it( END() );
    
    
    
    // Draw the touch points
    if ( (tx1 != 0) && ( tx2 != 0) )
    {
        // draw sample points
        ft801_api_cmd_append_it( COLOR_RGB(250, 250, 250) );
        ft801_api_cmd_append_it( COLOR_A(100) );
        ft801_api_cmd_append_it( BEGIN(FT_POINTS) ) ;
        ft801_api_cmd_append_it( POINT_SIZE(30*16) ) ;
        ft801_api_cmd_append_it( VERTEX2F( tx1, ty1) );
        ft801_api_cmd_append_it( VERTEX2F( tx2 , ty2) );
        ft801_api_cmd_append_it( END() );
    }
    

    
    
    ft801_api_cmd_append_it(DISPLAY()) ;
    ft801_api_cmd_append_it(CMD_SWAP);
    
    
    
    ft801_api_cmd_flush_it();
    
    return true ;
}


bool graphExample_doing( void * const data )
{
    
    uint16_t * myData ;
    if ( 1 == myData[0] )
    {
        myData[0] = 0 ;
        
        // read the tag
        //uint8_t tag = ft801_spi_rd8(REG_TOUCH_TAG);
        uint32_t tmp = ft801_spi_rd32(REG_TRACKER);
        uint8_t tag = tmp & 0xFF ;
        
        // back to menu
        if ( 200 == tag )
        {
            ft80x_gpu_eng_it_setActiveTask(TASK_ID1) ;
            return true ;
        }
        // detected touch on the graph field
        else if ( 201 == tag )
        {
            // read the xy of first touch and the second
            uint32_t tmp = ft801_spi_rd32(REG_CTOUCH_TOUCH0_XY);
            ty1 = tmp & 0xFFFF ;
            tx1 = tmp >> 16 ;
            
            // read the xy of the second touch
            tmp = ft801_spi_rd32(REG_CTOUCH_TOUCH1_XY);
            ty2 = tmp & 0xFFFF ;
            tx2 = tmp >> 16 ;
            
            
            
            // second touch detected
            if ( !((tx2 == ty2) && ( tx2 == -32768)) )
            {
                                               
                // calculate the direction
                x_scale = ( tx1 > tx2 ) ? 8*((tx1 - tx2)) : 8*((tx2 - tx1)) ;
                y_scale = ( ty1 > ty2 ) ? 4*((ty1 - ty2)) : 4*((ty2 - ty1)) ;
                
                tx1 = CONV_DIST(tx1) ;
                tx2 = CONV_DIST(tx2) ;
                ty1 = CONV_DIST(ty1) ;
                ty2 = CONV_DIST(ty2) ;
                
               
                
            }
            // only one touch
            else
            {
                tx1 = ty1 = tx2 = ty2 = 0;
            }
            
            
            ft80x_gpu_eng_it_setActiveTask(TASK_GRAPH) ;
            return true ;
            
        }
        
        
    }
    
    
    return true ;
}

bool graphExample_gpuit( const uint8_t itflags, void * const data )
{
    
    uint16_t * myData = data ;
    
    uint8_t mask = FT_INT_CONVCOMPLETE |  FT_INT_TAG /*| FT_INT_TOUCH */ ;
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

