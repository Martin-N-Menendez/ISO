/*==================[inclusions]=============================================*/
#include "board.h"
#include "os.h"
#include <strings.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

uint32_t tick_count = 0;
/*==================[internal functions declaration]=========================*/

void init_stack(uint32_t stack[],uint32_t stack_size_bytes,uint32_t * sp,task_type entry_point,void * arg);

void* idle(void* args);

/*==================[internal data definition]===============================*/

task_struct task_list[N_TASK];
uint32_t current_task;
uint32_t task_list_idx;

uint32_t stack_idle[STACK_SIZE/4];

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

void os_init(void){
	uint32_t i;

	for( i=0 ; i < N_TASK ; i++){
		task_list[i].id = 0;
		task_list[i].state = 0;
		task_list[i].stack_pointer = 0;
	}
	task_list_idx = 1;
	current_task = 0;
	task_create(stack_idle,STACK_SIZE,idle, (void*)0);
}

void task_create(uint32_t stack[],
				uint32_t stack_size_bytes,
				task_type entry_point,
				void * args){

	task_list[task_list_idx].id = task_list_idx;
	task_list[task_list_idx].state = RUNNING;
	init_stack(stack,stack_size_bytes,&task_list[task_list_idx].stack_pointer,entry_point,args);

	task_list_idx++;
}

void init_stack(uint32_t stack[],
				uint32_t stack_size_bytes,
				uint32_t * sp,
				task_type entry_point,
				void * arg)
{
	bzero(stack,stack_size_bytes);

	stack[stack_size_bytes/4-1] = 1 << 24;									/* xPSR.T = 1*/
	stack[stack_size_bytes/4-2] = (uint32_t)entry_point;					/* xPC */
	stack[stack_size_bytes/4-3] = (uint32_t)task_return_hook;				/* xLR */
	stack[stack_size_bytes/4-8] = (uint32_t)arg;							/* R0 */
	stack[stack_size_bytes/4-9] = 0xFFFFFFF9;								/* LR IRQ */

	/* Considero los otros 8 registros */
	void* stack_dir = &(stack[stack_size_bytes/4-17]);
	*sp = (uint32_t)stack_dir;
}

uint32_t get_next_context(uint32_t current_sp)
{
	uint32_t next_sp;

	/* If task == 0*/

	if ( current_task == 0){
		next_sp = task_list[1].stack_pointer;
		current_task = 1;
		return next_sp;
	}

	/* Save stack of current stack */
	task_list[current_task].stack_pointer = current_sp;

	/* Get next stack pointer and task */
	uint32_t next_task,idx;
	bool find_next_task = FALSE;

	for ( idx = 0 ; idx < (task_list_idx -1) ; idx++ ){
		uint32_t task_idx = ((current_task + idx) % (task_list_idx -1)) +1;
		task_state state = task_list[task_idx].state;
		switch(state){
		case READY:
			if( !find_next_task ){
				task_list[task_idx].state = RUNNING;
				find_next_task = TRUE;
				next_task = task_idx;
			}
			break;
		case RUNNING:
			task_list[task_idx].state = RUNNING;
			if( !find_next_task ){
				find_next_task = TRUE;
				next_task = task_idx;
			}
			break;
		case WAITING:
			task_list[task_idx].ticks --;
			if( task_list[task_idx].ticks == 0 ){
				if( !find_next_task ){
					task_list[task_idx].state = RUNNING;
					find_next_task = TRUE;
					next_task = task_idx;
				} else {
					task_list[task_idx].state = READY;
				}
			}
			break;
		case SUSPENDED:
			/* FALTA */
			break;
		default: break;
		}
	}

	if ( !find_next_task ){
		next_task = 1; // go to idle task
	}

	current_task = next_task;
	next_sp = task_list[next_task].stack_pointer;

	return next_sp;
}

void task_delay(uint32_t delay)
{
	task_list[current_task].state = WAITING;
	task_list[current_task].ticks = delay;
	while (task_list[current_task].ticks > 0){
		__WFI();
	}
}

void task_return_hook(void* ret_val)
{
	while(1){
		__WFI();
	}
}

void* idle(void* args){
	while(1){
		__WFI();
	}
}

void add_tick_count(void){
	tick_count++;
}

uint32_t get_tick_count(void){
	return tick_count;
}


/*==================[external functions definition]==========================*/

/*==================[end of file]============================================*/
