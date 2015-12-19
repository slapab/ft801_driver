#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "ft80x_task.h"

#include "ft80x_it_api_cmd.h"
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
    return true;
}


bool task1_gpuit( void * const data )
{
    return true;
}


