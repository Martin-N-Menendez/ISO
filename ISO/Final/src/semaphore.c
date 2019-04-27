
#include <stdlib.h>
#include "os.h"
#include "semaphore.h"

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef TRUE
#define TRUE 1
#endif

extern task_struct task_list[N_TASK];
extern uint32_t current_task;
//extern uint32_t task_list_idx;

void semaphore_create(semaphore_t* arg){
	arg->taken = TRUE;
	arg->ticks = -1;
}

void semaphore_give(semaphore_t* sem){
	sem->taken = FALSE;
}

void semaphore_take(semaphore_t* sem){

	if (sem != NULL && sem->taken){
		task_list[current_task].semaphore = sem;
		task_list[current_task].state =	WAITING;
		task_list[current_task].wait_state = WAIT_SEM;
		schedule();
	}
	sem->taken = TRUE;
}
