#ifndef D_LCD_TERMINAL_TASK_H
#define D_LCD_TERMINAL_TASK_H

#include <stdbool.h>
#include <stdint.h>



// Public functions for user:
void terminalTask_append_line( char * str ) ;


bool terminalTask_painting (void * const) ;

bool terminalTask_doing(void * const) ;

bool terminalTask_gpuit(const uint8_t, void * const) ;

#endif
