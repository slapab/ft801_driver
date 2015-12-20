#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "ft80x_task.h"

#include "ft80x_it_api_cmd.h"
#include "ft80x_engine_it.h"

#include "my_tasks.h"

#include "ft801_gpu.h"

bool task1_painting(void)
{
    ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
    
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(50, 50, 55));
    ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
    ft801_api_cmd_append_it(COLOR_RGB(0,100,250));
    ft801_api_cmd_text_it(240,136,30,FT_OPT_CENTER,"And what now?") ;

    ft801_api_cmd_append_it(DISPLAY()) ;
    ft801_api_cmd_append_it(CMD_SWAP);

    ft801_api_cmd_flush_it();
    
    return true ;
}

bool task1_doing( void * const data )
{
    uint8_t * myData = data ;
    
    if ( *myData == 2 )
    {
        // set new Active task
        ft80x_gpu_eng_it_setActiveTask(TASK_ID2) ;
        
    }
    
    return true;
}


bool task1_gpuit( const uint8_t itflags, void * const data )
{
    uint8_t * myData = data ;
    if ( itflags & FT_INT_TOUCH )
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


bool task2_painting(void)
{
    ft801_api_cmd_prepare_it( FT_RAM_CMD ) ;
    
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(0, 120, 255));
    ft801_api_cmd_append_it(CLEAR(1, 1, 1)) ;
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    ft801_api_cmd_text_it(240,136,30,FT_OPT_CENTER,"Screen two") ;

    ft801_api_cmd_append_it(DISPLAY()) ;
    ft801_api_cmd_append_it(CMD_SWAP);

    ft801_api_cmd_flush_it();
    
    return true ;
}

bool task2_doing( void * const data )
{
    uint8_t * myData = data ;
    
    if ( *myData == 1 )
    {
        // set new Active task
        ft80x_gpu_eng_it_setActiveTask(TASK_ID1) ;
        
    }
    
    return true;
}



bool task2_gpuit( const uint8_t itflags, void * const data )
{
    uint8_t * myData = data ;
    if ( itflags & FT_INT_TOUCH )
    {
        *myData = 1 ;
    }
    else
    {
        *myData = 0 ;
    }
    
    return true;
}

