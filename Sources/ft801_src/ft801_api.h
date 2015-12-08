#ifndef _FT801_API_H_
#define _FT801_API_H_

#include <stdbool.h>

/* 
*   Display List API
*/
#include "ft801_api_dl.h"

/*
*   COMMAND API
*/
#include "ft801_api_cmd.h"

void ft801_api_init_lcd( void ) ;

/**
*   To enable LCD module the 7 pin in GPIO must be
*   set as output and set to 1. If set to 0 then the LCD module is disabled.
*/
void ft801_api_enable_lcd( bool enable ) ;
bool ft801_api_is_enabled( void ) ;

/**
*   Enable or disable an interrupt pin
*/
void ft801_api_enable_it_pin( bool enable );
/**
*   Enable given interrupt source
*/
void ft801_api_enable_it_src( const uint8_t mask );
/**
*   Disable given interrupt source
*/
void ft801_api_disable_it_src( const uint8_t mask );

/**
*   Read the interrupt register
*   
*   @note   Each enabled flag will be cleared by this read!
*/
uint8_t ft801_api_read_it_flags( void );

/**
*   Send CTOUCH_TRANSFORM tables -> calibrate the Capacitive touchscreen
*/
void ft801_api_ctouch_adjust( void );


#endif