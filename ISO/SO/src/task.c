#include "task.h"
#include "os.h"
#include "task_queue.h"

task_struct task_list[N_TASK];

task_stack_t priority_queue[N_QUEUE];

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
	/*Inicializar stack de tarea*/
	init_stack(stack,stack_size_bytes,&task_list[task_list_idx].stack_pointer,entry_point,args);

	task_stack_push(&priority_queue[priority], task_list_idx);

	task_list_idx++;

	return TRUE;
}

void task_delay(uint32_t delay)
{
	task_list[current_task].state = WAITING;				/* Asignar estado WAITING */
	task_list[current_task].ticks = delay;					/* Guardar cantidad de ticks a esperar */
	//while (task_list[current_task].ticks > 0){
	//	__WFI();											/* Esperar interrupciones mientras tanto */
	//}
	schedule();
}

bool_t task_search_next( uint32_t* task_list_idx )
{
	//bool_t task_hit = FALSE;
	uint32_t i,j,L_queue,aux_idx;

	for ( i = 0 ; i < N_QUEUE; i++)
	{
		if(task_stack_empty(&priority_queue[i]) == FALSE)
		{
			L_queue = task_stack_size(&priority_queue[i]);
			for( j = 0 ; j < L_queue ; j++)
			{
				task_stack_pop(&priority_queue[i],&aux_idx);
				switch (task_list[aux_idx].state)
				{
					case READY:
					case RUNNING:
						(*task_list_idx) = aux_idx;
						task_stack_push(&priority_queue[i],aux_idx);
						return TRUE;
						break;
					case WAITING:
						break;
					default:
						os_error_hook(3);
						break;
				}
			}
		}
	}

	return FALSE;
}

void task_return_hook(void* args)
{
	while(1){
		__WFI();											/* Esperar interrupciones mientras tanto */
	}
}
