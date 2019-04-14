/*==================[inclusions]=============================================*/
#include "board.h"
#include "os.h"
#include <strings.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
uint32_t tick_count = 0;
/*==================[internal functions declaration]=========================*/

#define OFF 						0x0
/*==================[internal data definition]===============================*/

task_struct task_list[N_TASK];
uint32_t current_task;
uint32_t task_list_idx;

uint32_t stack_idle[STACK_SIZE/4];


/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

void os_init(void){	/* Inicializar las tareas y crear tarea idle */
	uint32_t i;

	for( i=0 ; i < N_TASK ; i++){	/* Inicializar tareas*/
		task_list[i].id = 0;
		task_list[i].state = 0;
		task_list[i].stack_pointer = 0;
	}
	task_list_idx = 1;
	current_task = 0;
	task_create(stack_idle,STACK_SIZE,idle,1, (void*)0);	/* Crear tarea Idle */
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

uint32_t get_next_context(uint32_t current_sp){ /* Intercambiador de contexto de tareas */
	uint32_t next_sp;

	if ( current_task == 0 ){ 	/* Si la tarea actual es idle */
		next_sp = task_list[1].stack_pointer;
		current_task = 1;
		return next_sp;
	}

	/* Guardar stack de tarea actual */
	task_list[current_task].stack_pointer = current_sp;

	/* Obtener proximo SP y tarea */
	uint32_t next_task,idx;
	bool find_next_task = FALSE;

	for ( idx = 0 ; idx < (task_list_idx -1) ; idx++ ){
		uint32_t task_idx = ((current_task + idx) % (task_list_idx -1)) +1;
		task_state state = task_list[task_idx].state;
		semaphore_t* sem;
		switch(state){
		case READY:											/* Estado READY */
			if( !find_next_task ){
				task_list[task_idx].state = RUNNING;		/* Cambiar estado */
				find_next_task = TRUE;						/* Buscar proximo */
				next_task = task_idx;						/* Proxima tarea */
			}
			break;
		case RUNNING:										/* Estado RUNNING */
			task_list[task_idx].state = RUNNING;			/* Cambiar estado */
			if( !find_next_task ){
				find_next_task = TRUE;						/* Buscar proximo */
				next_task = task_idx;						/* Proxima tarea */
			}
			break;
		case WAITING:										/* Estado WAITING */
			switch(task_list[task_idx].wait_state){
			case WAIT_TICKS:
				task_list[task_idx].ticks --;					/* Decremento los ticks */
				if( task_list[task_idx].ticks == 0 ){			/* Se espero suficiente? */
					task_list[task_idx].state = READY;			/* Cambiar estado */
					/* PRIO */
				}
				break;
			case WAIT_SEM:
				sem = task_list[task_idx].semaphore;
				if (sem != NULL && sem->taken == FALSE){
					task_list[task_idx].state = READY;
					/* PRIO */
				}
				break;
			default: break;
			}
			break;
		case SUSPENDED:										/* Estado SUSPENDED */
			/* FALTA */
			break;
		default: break;
		}
	}

	if ( !find_next_task ){									/* Si no hay proximas tareas */
		next_task = 1; 										/* Tarea 1 = Idle */
	}

	current_task = next_task;								/* Asignar tarea actual */
	next_sp = task_list[next_task].stack_pointer;			/* Asignar SP */

	return next_sp;
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


/*==================[external functions definition]==========================*/
/*
void gpio_obtain_pin_config( gpioMap_t pin, int8_t* pin_name_port,int8_t* pin_name_pin, int8_t* func, int8_t* gpio_port,int8_t* gpio_pin){

	*pin_name_port = gpio_pins_config[pin].pin_name.port;
	*pin_name_pin  = gpio_pins_config[pin].pin_name.pin;
	*func          = gpio_pins_config[pin].func;
	*gpio_port     = gpio_pins_config[pin].gpio.port;
	*gpio_pin      = gpio_pins_config[pin].gpio.pin;

}

bool gpioRead( gpioMap_t pin ){

	bool ret_val = OFF;

	int8_t pin_name_port = 0;
	int8_t pin_name_pin = 0;

	int8_t func = 0;

	int8_t gpio_port = 0;
	int8_t gpio_pin = 0;


	gpio_obtain_pin_config( pin, &pin_name_port, &pin_name_pin, &func, &gpio_port,&gpio_pin);

	ret_val = (bool) Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, gpio_port, gpio_pin);

	return ret_val;
}

void led_set( gpioMap_t LEDNumber, bool On ){

	Board_LED_Set(LEDNumber-LEDR,On);
}
*/

/*==================[end of file]============================================*/
