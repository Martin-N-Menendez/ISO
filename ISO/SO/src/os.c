/*==================[inclusions]=============================================*/
#include "board.h"
#include "os.h"
#include "sapi.h"
#include "task.h"
#include "semaphore.h"
#include <strings.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
static uint32_t tick_count = 0;
/*==================[internal functions declaration]=========================*/

uint32_t ready_list[PRIORITY_HIGH][N_TASK];
uint32_t ready_count[PRIORITY_HIGH];
/*==================[internal data definition]===============================*/

//os_state_t os_state = OS_INIT;

extern task_struct task_list[N_TASK];
extern task_struct idle_task;

extern uint32_t stack_idle[TASK_STACK_SIZE/4];

uint32_t current_task;
uint32_t task_list_idx;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

void os_init(void){	/* Inicializar las tareas y crear tarea idle */
	uint32_t i;

	//task_create(stack_idle,TASK_STACK_SIZE,idle,PRIORITY_IDLE, (void*)0);	// Crear tarea Idle

	idle_task.state = RUNNING;
	idle_task.stack_pointer = stack_idle;
	idle_task.priority = PRIORITY_IDLE;

	init_stack(stack_idle,TASK_STACK_SIZE,&idle_task.stack_pointer,idle,(void *)0x99999999);

	/* inicializo contextos iniciales de cada tarea */
	for (i = 0; i < N_TASK; i++) {
		add_ready(task_list[i].priority, i);
	}

	task_list_idx = 0;
	current_task = -1;

	schedule();
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
	//tick_count++;
	task_delay_update();
	schedule();
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

int32_t get_next_context(int32_t current_context)
{
	uint32_t returned_stack;

	if(current_task == -1)
	{
		idle_task.state = READY;
		returned_stack = current_context;
	}
	/* guardo contexto actual si es necesario */
	if (current_task == IDLE_TASK) {
		idle_task.stack_pointer = current_context;
		idle_task.state = READY;
	}
	else if (current_task < N_TASK) {
		task_list[current_task].stack_pointer = current_context;
		if (task_list[current_task].state == RUNNING) {
			task_list[current_task].state = READY;
			add_ready(task_list[current_task].priority, current_task);
		}
	}
	/* decido cuál va a ser el contexto siguiente a ejecutar */
	task_priority_t p;
	for (p = PRIORITY_HIGH; p > PRIORITY_IDLE; p--) {
		if (ready_count[p-1] > 0) {
			current_task = ready_list[p-1][0];
			remove_ready(p, current_task);
			task_list[current_task].state = RUNNING;
			returned_stack = task_list[current_task].stack_pointer;
			break;
		}
	}
	if (p == PRIORITY_IDLE) {
		current_task = IDLE_TASK;
		idle_task.state = RUNNING;
		returned_stack = idle_task.stack_pointer;
		//returned_stack = task_list[current_task].stack_pointer;
	}
	return returned_stack;
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
	tick_count++;											/* Incrementar cantidad de ticks */
}

uint32_t get_tick_count(void){
	return tick_count;										/* Devolver cantidad de ticks */
}

void add_ready(task_priority_t prio, uint32_t id)
{
	ready_list[prio-1][ready_count[prio-1]] = id;
	ready_count[prio-1]++;
}

void remove_ready(task_priority_t prio, uint32_t id)
{
	uint32_t i;
	for (i=0; i<ready_count[prio-1]; i++) {
		if (ready_list[prio-1][i] == id) {
			break;
		}
	}
	if (i < ready_count[prio-1]) {
		uint32_t j;
		for (j=i; j<(ready_count[prio-1]-1); j++) {
			ready_list[prio-1][j] = ready_list[prio-1][j+1];
		}
		ready_count[prio-1]--;
	}
}

void task_delay_update(void)
{
	uint32_t i;
	semaphore_t* sem;

	tick_count++;
	for (i = 0; i < N_TASK; i++) {
		if(task_list[i].state == WAITING)
		{
			switch(task_list[i].wait_state)
			{
			case WAIT_TICKS:
				if(task_list[i].ticks > 0)
				{
					task_list[i].ticks--;
					if (task_list[i].ticks == 0)
					{
						task_list[i].state = READY;
						add_ready(task_list[i].priority, i);
					}
				}
				break;
			case WAIT_SEM:
				sem = task_list[i].semaphore;
				if(sem != NULL && sem->taken == FALSE)
				{
					task_list[i].state = READY;
					add_ready(task_list[i].priority, i);
				}
				break;

			default:
				os_error_hook(1);
				break;
			}
		}
	}




}


/*==================[external functions definition]==========================*/

/*==================[end of file]============================================*/
