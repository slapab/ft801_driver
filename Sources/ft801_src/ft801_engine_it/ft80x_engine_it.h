#ifndef _FT80X_ENGINE_IT_H_
#define _FT80X_ENGINE_IT_H_

#include "ft80x_task.h"
#include "ft801_gpu.h"

void ft80x_gpu_eng_it_rountine(void);
void ft80x_gpu_eng_it_looper(void) ;


void ft80x_gpu_eng_it_init( void );
bool ft80x_gpu_eng_it_reg_task( FT80xTask_TypeDef * const pTask ) ;
void ft80x_gpu_eng_it_setActiveTask( const uint16_t task_id ) ;



#endif

