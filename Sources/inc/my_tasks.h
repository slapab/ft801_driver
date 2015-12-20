#ifndef _MY_TASKS_H_
#define _MY_TASKS_H_

#include <stdbool.h>
#include <stdint.h>

#define TASK_ID1 1
bool task1_painting(void);
bool task1_doing( void * const data );
bool task1_gpuit( const uint8_t itflags, void * const data );


#define TASK_ID2 2
bool task2_painting(void);
bool task2_doing( void * const data );
bool task2_gpuit( const uint8_t itflags, void * const data );



#endif
