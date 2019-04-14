#include "task.h"
#include "os.h"

extern task_struct task_list[N_TASK];

void task_create(uint32_t stack[],
				uint32_t stack_size_bytes,
				task_type entry_point,
				uint8_t priority,
				void * args){

	task_list[task_list_idx].id = task_list_idx;	/* Asignar indice de tarea */
	task_list[task_list_idx].state = READY;		/* Asignar estado de tarea */
	task_list[task_list_idx].priority = priority;
	/*Inicializar stack de tarea*/
	init_stack(stack,stack_size_bytes,&task_list[task_list_idx].stack_pointer,entry_point,args);

	//task_priority_list(&task_queue, &task_list[task_list_idx]);

	task_list_idx++;
}

void task_delay(uint32_t delay)
{
	task_list[current_task].state = WAITING;				/* Asignar estado WAITING */
	task_list[current_task].wait_state = WAIT_TICKS;
	task_list[current_task].ticks = delay;					/* Guardar cantidad de ticks a esperar */
	//while (task_list[current_task].ticks > 0){
	//	__WFI();											/* Esperar interrupciones mientras tanto */
	//}
	schedule();
}

void task_return_hook(void* args)
{
	while(1){
		__WFI();											/* Esperar interrupciones mientras tanto */
	}
}
