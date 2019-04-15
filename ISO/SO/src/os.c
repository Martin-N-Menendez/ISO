/*==================[inclusions]=============================================*/
#include "board.h"
#include "os.h"
#include "sapi.h"
#include "task.h"
#include "task_queue.h"
#include <strings.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
uint32_t tick_count = 0;
/*==================[internal functions declaration]=========================*/

//#define OFF 						0x0
/*==================[internal data definition]===============================*/

os_state_t os_state = OS_INIT;

extern task_struct task_list[N_TASK];
extern task_stack_t priority_queue[N_QUEUE];

extern uint32_t stack_idle[TASK_STACK_SIZE/4];
extern uint32_t stack1[TASK_STACK_SIZE/4];
extern uint32_t stack2[TASK_STACK_SIZE/4];
extern uint32_t stack3[TASK_STACK_SIZE/4];

uint32_t current_task;
uint32_t task_list_idx;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

void os_init(void){	/* Inicializar las tareas y crear tarea idle */
	//uint32_t i;

	task_list[0].id = 0;
	task_list[0].state = READY;
	task_list[0].stack_pointer = stack_idle;
	task_list[0].ticks = 0;
	task_list[0].priority = PRIORITY_IDLE;

	task_list_idx = 0;
	current_task = 1;

	task_create(stack_idle,TASK_STACK_SIZE,idle,task_list[0].priority, (void*)0);	// Crear tarea Idle
}

void schedule(void){	/* Programador */
	/* Instruction Synchronization Barrier: Aseguramos que se ejecuten todas las instrucciones en el pipleine*/
	__ISB();
	/* Data Synchronization Barrier: Aseguramos que se completen todos los accesos a memoria */
	__DSB();
	/* Activo PendSV pra llevar el cambio de contexto */
	SCB -> ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void SysTick_Handler(void)
{
	//add_tick_count();
	//os_update_delay();
	schedule();
}

void os_queue_init(void){
	uint32_t i;
	i = N_QUEUE;
	for( i = 0 ; i < N_QUEUE ; i++ ){
		task_stack_init(&priority_queue[i]);
	}
}

void init_stack(uint32_t stack[],
				uint32_t stack_size_bytes,
				uint32_t * sp,
				task_type entry_point,
				void * arg)
{
	bzero(stack,stack_size_bytes);	 										/* Vaciar stack */

	stack[stack_size_bytes/4-1] = 1 << 24;									/* xPSR.T = 1*/
	stack[stack_size_bytes/4-2] = (uint32_t)entry_point;					/* xPC */
	stack[stack_size_bytes/4-3] = (uint32_t)task_return_hook;				/* xLR */
	stack[stack_size_bytes/4-8] = (uint32_t)arg;							/* R0 */
	stack[stack_size_bytes/4-9] = 0xFFFFFFF9;								/* LR IRQ */

	/* Considero los otros 8 registros */
	void* stack_dir = &(stack[stack_size_bytes/4-17]);
	*sp = (uint32_t)stack_dir;
}

uint32_t get_next_context(uint32_t current_sp){ /* Intercambiador de contexto de tareas */

	uint32_t next_sp;

	bool_t task_hit = FALSE;
	uint32_t task_index;

	switch (os_state) {
	case OS_INIT:
		task_hit = task_search_next(&task_index);
		if (task_hit == TRUE) {
			os_state = OS_TASK;
			task_list[task_index].state = RUNNING;
			next_sp = task_list[task_index].stack_pointer;
			current_task = task_index;
		} else {
			os_state = OS_IDLE;
			next_sp = task_list[0].stack_pointer;
		}
		break;
	case OS_TASK:
		task_list[current_task].stack_pointer = current_sp;
		if (task_list[current_task].state == RUNNING) {
			task_list[current_task].state = READY;
		}
		task_hit = task_search_next(&task_index);
		if (task_hit == TRUE) {
			os_state = OS_TASK;
			task_list[task_index].state = RUNNING;
			next_sp = task_list[task_index].stack_pointer;
			current_task = task_index;
		} else {
			os_state = OS_IDLE;
			next_sp = task_list[0].stack_pointer;
		}
		break;
	case OS_IDLE:
		task_list[0].stack_pointer = current_sp;
		task_hit = task_search_next(&task_index);
		if (task_hit == TRUE) {
			os_state = OS_TASK;
			task_list[task_index].state = RUNNING;
			next_sp = task_list[task_index].stack_pointer;
			current_task = task_index;
		} else {
			os_state = OS_IDLE;
			next_sp = task_list[0].stack_pointer;
		}
		break;
	default:
		os_error_hook(1);
		break;
	}

	return next_sp;
}

void os_update_delay(void)
{
	uint32_t i;
		for (i = 0; i < task_list_idx ; i++) {
			switch (task_list[i].state) {
			case RUNNING:
			case READY:
				break;
			case WAITING:
				task_list[i].ticks -= 1;
				if (task_list[i].ticks == 0) {
					//Pongo la tarea en ready y la pongo en la cola de prioridad que le corresponde
					task_list[i].state = READY;
					task_stack_push(&priority_queue[task_list[i].priority], i);
				}
				break;
			default:
				os_error_hook(2);
				break;
			}
	}
}

void os_error_hook(int i){
	while(1){
		__WFI();											/* Esperar interrupciones mientras tanto */
	}
}

void* idle(void* args){
	while(1){
		__WFI();											/* Esperar interrupciones mientras tanto */
	}
}

void add_tick_count(void){
	os_update_delay();
	tick_count++;											/* Incrementar cantidad de ticks */
}

uint32_t get_tick_count(void){
	return tick_count;										/* Devolver cantidad de ticks */
}


/*==================[external functions definition]==========================*/

/*==================[end of file]============================================*/
