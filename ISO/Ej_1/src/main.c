/*==================[inclusions]=============================================*/
#include "board.h"
#include <stdlib.h>
#include <strings.h>
#include "os.h"
/*==================[macros and definitions]=================================*/



//#define STACK_SIZE 1024
#define DELAY_MS 1000
#define LED 2	//#define LED LEDR

//typedef void *(*task_type)(void *);

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
}

/*==================[external functions definitibon]==========================*/

void * task1(void* arg)
{

	while(1){
		Board_LED_Toggle(LED+1);
		task_delay(200);
	}
	return NULL;
}

void * task2(void* arg)
{

	while(1){
		Board_LED_Toggle(LED+3);
		task_delay(300);
	}
	return NULL;
}


/*
uint32_t get_next_context_2(uint32_t current_action)
{
	switch (current_task){
		case WAITING:
			// si release > ready
			break;
		case READY:
			// si start > running
			break;
		case SUSPENDED:
			// si activate > ready
			break;
		case RUNNING:
			// si wait > waiting
			// si preempt > ready
			// si terminate > suspended
			break;
		default:
			break;

	return next_sp;
}
*/

/*
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
*/


/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){

	/* ------------- INICIALIZACIONES ------------- */

	os_init();
	task_create(stack1,STACK_SIZE,task1,(void *)0x11223344);
	task_create(stack2,STACK_SIZE,task2,(void *)0x55667788);

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
