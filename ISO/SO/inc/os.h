#ifndef OS_H
#define OS_H

#include <stdint.h>
#include <stdlib.h>
#include "task.h"
#include "semaphore.h"

#define bool _Bool

typedef enum{
	OS_INIT = 0,
	OS_TASK,
	OS_IDLE
} os_state_t;


void os_init(void);
void os_queue_init(void);
void os_update_delay(void);
void schedule(void);
void SysTick_Handler(void);

void init_stack(uint32_t stack[],
				uint32_t stack_size_bytes,
				uint32_t * sp,
				task_type entry_point,
				void * arg);
uint32_t get_next_context(uint32_t current_sp);

void* idle(void* args);
void add_tick_count(void);
uint32_t get_tick_count(void);
void led_set( uint8_t LEDNumber, bool On );
void os_error_hook(int i);

#endif
