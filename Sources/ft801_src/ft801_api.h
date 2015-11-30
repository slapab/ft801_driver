#ifndef _FT801_API_H_
#define _FT801_API_H_

#include <stdbool.h>

void ft801_api_init_lcd( void ) ;

/**
*   To enable LCD module the 7 pin in GPIO must be
*   set as output and set to 1. If set to 0 then the LCD module is disabled.
*/
void ft801_api_enable_lcd( bool enable ) ;
bool ft801_api_is_enabled( void ) ;


#endif