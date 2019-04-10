#ifndef TASK_H
#define TASK_H

#include "board.h"
#include <stdint.h>
#include <stdlib.h>
#include "semaphore.h"

#define N_TASK 10
#define STACK_SIZE 1024


extern uint32_t current_task;
extern uint32_t task_list_idx;

typedef void *(*task_type)(void*);

typedef enum {
	RUNNING = 1,
	READY,
	WAITING,
	SUSPENDED
} task_state;

typedef enum {WAIT_TICKS = 1,
			  WAIT_SEM
} task_waiting_state;

typedef struct {
	uint32_t id;					/* Task ID */
	task_state state;				/* Task State */
	uint32_t stack_pointer;			/* Task Stack Pointer */
	uint32_t ticks;					/* Ticks*/
	uint8_t priority;
	semaphore_t* semaphore;			/* Semaphore */
	task_waiting_state wait_state; 	/* Tick or Sem?*/
} task_struct;


void task_create(uint32_t stack[],
				uint32_t stack_size_bytes,
				task_type entry_point,
				uint8_t priority,
				void * args);

void task_delay(uint32_t delay);
void task_return_hook(void* args);

#endif
