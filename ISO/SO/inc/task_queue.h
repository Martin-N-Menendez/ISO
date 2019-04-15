#ifndef TASK_QUEUE
#define TASK_QUEUE

#include <stdint.h>
#include "sapi.h"

#ifdef __cplusplus
extern "C" {
#endif

//Mismo numero del stack size
#define N_STACK 10

typedef struct {
	uint32_t data[N_STACK];
	uint32_t read_pointer;
	uint32_t write_pointer;
	uint32_t size;
	bool_t	 stack_initializated;
} task_stack_t;


void	 	task_stack_init 				(task_stack_t *	task_stack);
bool_t 		task_stack_pop 					(task_stack_t * task_stack,uint32_t * data );
bool_t 		task_stack_push					(task_stack_t * task_stack,uint32_t   data );
bool_t	 	task_stack_empty 		(task_stack_t * task_stack);
bool_t		task_stack_is_full  		(task_stack_t * task_stack);
uint32_t	task_stack_size 		(task_stack_t * task_stack);
bool_t 		tack_stack_remove_item	(task_stack_t * task_stack,uint32_t item);

#ifdef __cplusplus
}
#endif

#endif
