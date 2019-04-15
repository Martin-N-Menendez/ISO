#include "task.h"
#include "os.h"


#define INVALID_TASK		((uint32_t)-1)

task_struct task_list[N_TASK];
task_struct idle_task;


uint32_t stack_idle[TASK_STACK_SIZE/4];
uint32_t stack1[TASK_STACK_SIZE/4];
uint32_t stack2[TASK_STACK_SIZE/4];
uint32_t stack3[TASK_STACK_SIZE/4];


bool_t task_create(uint32_t stack[],
				uint32_t stack_size_bytes,
				task_type entry_point,
				task_priority_t priority,
				void * args){

	if(task_list_idx >= N_TASK || stack_size_bytes < MIN_STACK_SIZE || stack_size_bytes % 4)
		return FALSE;


	task_list[task_list_idx].id = task_list_idx;	/* Asignar indice de tarea */
	task_list[task_list_idx].state = READY;		/* Asignar estado de tarea */
	task_list[task_list_idx].priority = priority;
	task_list[task_list_idx].ticks = 0;
	task_list[task_list_idx].stack_pointer = stack;
	task_list[task_list_idx].wait_state = WAIT_TICKS;
	/*Inicializar stack de tarea*/
	init_stack(stack,stack_size_bytes,&task_list[task_list_idx].stack_pointer,entry_point,args);

	//task_stack_push(&priority_queue[priority], task_list_idx);

	task_list_idx++;

	return TRUE;
}

void task_delay(uint32_t delay)
{

		if ((delay != 0) && (current_task != INVALID_TASK)) {
			task_list[current_task].state = WAITING;
			task_list[current_task].ticks = delay;
			schedule();
		}

}


void task_return_hook(void* args)
{
	while(1){
		__WFI();											/* Esperar interrupciones mientras tanto */
	}
}
