/*==================[inclusions]=============================================*/
#include "board.h"
#include <stdlib.h>
#include <strings.h>
/*==================[macros and definitions]=================================*/



#define STACK_SIZE 1024
#define DELAY_MS 1000
#define LED 2	//#define LED LEDR

typedef void *(*task_type)(void *);

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void initHardware(void);
//static void pausems(uint32_t t);
static void * task1(void * param);
static void * task2(void * param);

/*==================[internal data definition]===============================*/

//static uint32_t pausems_count;

/*==================[external data definition]===============================*/

uint32_t stack1[STACK_SIZE/4];
uint32_t stack2[STACK_SIZE/4];

uint32_t sp1,sp2;

uint32_t current_task;

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) -1);
}

void schedule(void)
{
	__ISB();
	__DSB();

	SCB -> ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/*
static void pausems(uint32_t t)
{
	pausems_count = t;
	while (pausems_count != 0){
		__WFI();
	};
}
*/
/*==================[external functions definition]==========================*/

void * task1(void* arg)
{
	uint32_t i;
	while(1){
		//__WFI();
		//os_delay(100);
		for (i = 0; i < 200000; i++);
		Board_LED_Toggle(LED+2);

	}
	return NULL;
}

void * task2(void* arg)
{
	uint32_t j;
	while(1){
		//__WFI();
		//os_delay(100);
		for (j = 0; j < 300000; j++);
		Board_LED_Toggle(LED+3);
	}
	return NULL;
}

void SysTick_Handler(void)
{
	schedule();
}

void task_return_hook(void* ret_val)
{
	while(1){
		__WFI();
	}
}

uint32_t get_next_context(uint32_t current_sp)
{
	uint32_t next_sp;

	// Es mejor contar TICKS aca!

	switch (current_task){
		case 0:
			next_sp = sp1;
			current_task = 1;
			break;
		case 1:
			sp1 = current_sp;
			next_sp = sp2;
			current_task = 2;
			break;
		case 2:
			sp2 = current_sp;
			next_sp = sp1;
			current_task = 1;
			break;
		default:
			while(1){
				__WFI();
			}
			break;
	}
	return next_sp;
}

void init_stack(uint32_t stack[],
				uint32_t stack_size_bytes,
				uint32_t * sp,
				task_type entry_point,
				void * arg)
{
	bzero(stack,stack_size_bytes);

	stack[stack_size_bytes/4-1] = 1 << 24;								/* xPSR.T = 1*/
	stack[stack_size_bytes/4-2] = (uint32_t)entry_point;					/* xPC */
	stack[stack_size_bytes/4-3] = (uint32_t)task_return_hook;				/* xLR */
	stack[stack_size_bytes/4-8] = (uint32_t)arg;							/* R0 */
	stack[stack_size_bytes/4-9] = 0xFFFFFFF9;								/* LR IRQ */

	/* Considero los otros 8 registros */
	*sp = (uint32_t)&(stack[stack_size_bytes/4-17]);
}


/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){

	/* ------------- INICIALIZACIONES ------------- */
	init_stack(stack1,STACK_SIZE,&sp1,task1,(void *)0x11223344);
	init_stack(stack2,STACK_SIZE,&sp2,task2,(void *)0x55667788);

	initHardware(); /* Inicializar la placa */

	while(1) {  /* ------------- REPETIR POR SIEMPRE ------------- */
		Board_LED_Toggle(LED);
		//pausems(DELAY_MS);
		__WFI();
	}

	/* NO DEBE LLEGAR NUNCA AQUI*/
	return 0 ;
}

/*==================[end of file]============================================*/
