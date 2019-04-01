#ifndef OS_H
#define OS_H

#include <stdint.h>

#define N_TASK 10
#define STACK_SIZE 1024

#define bool _Bool

typedef enum {
	RUNNING = 1,
	READY,
	WAITING,
	SUSPENDED
} task_state;

typedef struct {
	uint32_t id;					/* Task ID */
	task_state state;				/* Task State */
	uint32_t stack_pointer;			/* Task Stack Pointer */
	uint32_t ticks;					/* Ticks*/
} task_struct;

typedef void *(*task_type)(void*);

void os_init(void);
void schedule(void);
void SysTick_Handler(void);
void task_create(uint32_t stack[],
				uint32_t stack_size_bytes,
				task_type entry_point,
				void * args);
void init_stack(uint32_t stack[],
				uint32_t stack_size_bytes,
				uint32_t * sp,
				task_type entry_point,
				void * arg);
uint32_t get_next_context(uint32_t current_sp);
void task_delay(uint32_t delay);
void task_return_hook(void* args);
void* idle(void* args);
void add_tick_count(void);
uint32_t get_tick_count(void);

#endif
