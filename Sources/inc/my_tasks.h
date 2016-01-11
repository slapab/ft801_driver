#ifndef _MY_TASKS_H_
#define _MY_TASKS_H_

#include <stdbool.h>
#include <stdint.h>

#include "lcd_tasks_ids.h"
#include "terminal_task.h"
#include "demo_tasks.h"



#define TASK_ID1 1
bool task1_painting(void * const data);
bool task1_doing( void * const data );
bool task1_gpuit( const uint8_t itflags, void * const data );



#define TASK_ID2 2
bool task2_painting(void * const data);
bool task2_doing( void * const data );
bool task2_gpuit( const uint8_t itflags, void * const data );



#define TASK_KEYBOARD 3 
bool keyboardTask_painting( void * const );
bool keyboardTask_doing( void * const );
bool keyboardTask_gpuit( const uint8_t, void * const ) ;



#define TASK_GRAPH 4
bool graphExample_painting( void * const );
bool graphExample_doing( void * const );
bool graphExample_gpuit( const uint8_t, void * const ) ;


#endif
