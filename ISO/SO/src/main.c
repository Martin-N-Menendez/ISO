/*==================[inclusions]=============================================*/
#include "board.h"
#include <stdlib.h>
#include <strings.h>
#include "os.h"
#include "sapi.h"
#include "task.h"
#include "semaphore.h"
/*==================[macros and definitions]=================================*/

#define EJ0

typedef enum {UP,DOWN} button_state;

semaphore_t xSem;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void initHardware(void);

#ifdef EJ0
static void * task1(void * param);
static void * task2(void * param);
static void * task3(void * param);
#endif

//extern bool gpioRead( gpioMap_t pin );

/*==================[internal data definition]===============================*/

//static uint32_t pausems_count;

/*==================[external data definition]===============================*/

extern uint32_t stack1[TASK_STACK_SIZE/4];
extern uint32_t stack2[TASK_STACK_SIZE/4];
extern uint32_t stack3[TASK_STACK_SIZE/4];

uint32_t led_on_time_tick;
uint32_t launched = 0;

uint32_t current_task;

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) -1);
	SysTick_Config(SystemCoreClock / 1000);
}

#ifdef EJ0
void * task1(void* arg)
{
	while(1){
		gpioToggle(LED1);
		task_delay(50);
	}
	return NULL;
}

void * task2(void* arg)
{
	while(1){
		gpioToggle(LED2);
		task_delay(100);
	}
	return NULL;
}

void * task3(void* arg)
{
	while(1){
		gpioToggle(LED3);
		task_delay(250);
	}
	return NULL;
}

#endif

/*==================[external functions definition]==========================*/
#ifdef EJ1
void* button_task(void* taskParam)
{

	button_state state = UP;
	uint32_t on_time;

	while(1){
		task_delay(10);

		if(!gpioRead(TEC1) && state == UP){
			task_delay(20);
			if(!gpioRead(36)){
				state = DOWN;
				on_time = get_tick_count();
			}
		}

		if(gpioRead(TEC1) && state == DOWN){
			task_delay(20);
			if(gpioRead(36)){
				state = UP;
				on_time = get_tick_count() - on_time;
				led_on_time_tick = on_time;
				semaphore_give(&xSem);
			}
		}
	}
}

void* led_task(void* taskParam){
	while(1){
		semaphore_take(&xSem);
		//Board_LED_Set(LED,ON);
		gpioToggle(LEDR);
		task_delay(led_on_time_tick);
		//Board_LED_Set(LED,OFF);
		gpioToggle(LEDR);
	}
}

#endif

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){

	/* ------------- INICIALIZACIONES ------------- */

	initHardware(); /* Inicializar la placa */

	#ifdef EJ0
	task_create(stack1,TASK_STACK_SIZE,task1,PRIORITY_LOW,(void *)0x11223344);
	task_create(stack2,TASK_STACK_SIZE,task2,PRIORITY_LOW,(void *)0x55667788);
	task_create(stack3,TASK_STACK_SIZE,task3,PRIORITY_LOW,(void *)0x22446688);
	#endif

	#ifdef EJ1
	semaphore_create(&xSem);
	task_create(stack1,TASK_STACK_SIZE,button_task,PRIORITY_LOW,(void *)0x11223344);
	task_create(stack2,TASK_STACK_SIZE,led_task,PRIORITY_LOW,(void *)0x55667788);
	#endif

	os_init();

	while(1) {  /* ------------- REPETIR POR SIEMPRE ------------- */
		//Board_LED_Toggle(LEDB);
		gpioToggle(LEDR);
		//task_delay(500);
		//Board_LED_Set(LED,OFF);
		__WFI();
	}

	/* NO DEBE LLEGAR NUNCA AQUI*/
	return 0 ;
}

/*==================[end of file]============================================*/
