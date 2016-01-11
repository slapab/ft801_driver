#ifndef D_LCD_DEMO_TASKS_H
#define D_LCD_DEMO_TASKS_H

#include "lcd_tasks_ids.h"
#include <stdbool.h>
#include <stdint.h>

bool demoMenuTask_doing(void * const data);
bool demoMenuTask_painting (void * const data);
bool demoMenuTask_gpuit(const uint8_t itflags, void * const data);


bool demoGraphTask_doing(void * const data);
bool demoGraphTask_painting (void * const data);
bool demoGraphTask_gpuit(const uint8_t itflags, void * const data);

bool demoSettTask_doing(void * const data);
bool demoSettTask_painting (void * const data);
bool demoSettTask_gpuit(const uint8_t itflags, void * const data);



#endif