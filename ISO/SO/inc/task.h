#ifndef TASK_H
#define TASK_H

#include "board.h"
#include "sapi.h"
#include <stdint.h>
#include <stdlib.h>
#include "semaphore.h"

#define N_TASK 				4
#define N_QUEUE				3
#define TASK_STACK_SIZE 	1024
#define MIN_STACK_SIZE		32
#define IDLE_STACK_SIZE		512

#define IDLE_TASK			N_TASK-1

extern uint32_t current_task;
extern uint32_t task_list_idx;

typedef void *(*task_type)(void*);

typedef enum {
	RUNNING = 1,
	READY,
	WAITING,
	SUSPENDED
} task_state;

typedef enum {
	PRIORITY_IDLE = 0,
	PRIORITY_LOW,
	PRIORITY_MEDIUM,
	PRIORITY_HIGH
} task_priority_t;

typedef enum {WAIT_TICKS = 1,
			  WAIT_SEM
} task_waiting_state;

typedef struct {
	uint32_t id;					/* Task ID */
	task_state state;				/* Task State */
	uint32_t stack_pointer;			/* Task Stack Pointer */
	uint32_t ticks;					/* Ticks*/
	task_priority_t priority;
	semaphore_t* semaphore;			/* Semaphore */
	task_waiting_state wait_state; 	/* Tick or Sem?*/
} task_struct;


bool_t task_create(uint32_t stack[],
				uint32_t stack_size_bytes,
				task_type entry_point,
				task_priority_t priority,
				void * args);

void task_delay(uint32_t delay);
void task_return_hook(void* args);
bool_t task_search_next( uint32_t* task_list_idx );

#endif
