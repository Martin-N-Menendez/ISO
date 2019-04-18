/*==================[inclusions]=============================================*/
#include "board.h"
#include <stdlib.h>
#include <strings.h>
#include "os.h"
#include "sapi.h"
#include "task.h"
#include "semaphore.h"
#include "buttons.h"
/*==================[macros and definitions]=================================*/

#define EJ1_bis

//typedef enum {UP,DOWN} button_state;

semaphore_t xSem;

extern volatile Buttons_t button_list[N_BUTTON];
extern gpioMap_t buttons_index[];

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void initHardware(void);

#ifdef EJ0
static void * task1(void * param);
static void * task2(void * param);
static void * task3(void * param);
static void * task4(void * param);
#endif

/*==================[internal data definition]===============================*/


/*==================[external data definition]===============================*/

extern uint32_t stack1[TASK_STACK_SIZE/4];
extern uint32_t stack2[TASK_STACK_SIZE/4];
extern uint32_t stack3[TASK_STACK_SIZE/4];
extern uint32_t stack4[TASK_STACK_SIZE/4];

uint32_t led_on_time[N_BUTTON] = {0,0,0,0};
//gpioMap_t buttons[] = { TEC1,TEC2,TEC3,TEC4 };

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
		gpioToggle(LEDG);
		task_delay(50);
	}
	return NULL;
}

void * task2(void* arg)
{
	while(1){
		gpioToggle(LED1);
		task_delay(100);
	}
	return NULL;
}

void * task3(void* arg)
{
	while(1){
		gpioToggle(LED2);
		task_delay(250);
	}
	return NULL;
}

void * task4(void* arg)
{
	while(1){
		gpioToggle(LED3);
		task_delay(500);
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
			if(!gpioRead(TEC1)){
				state = DOWN;
				on_time = get_tick_count();
			}
		}

		if(gpioRead(TEC1) && state == DOWN){
			task_delay(20);
			if(gpioRead(TEC1)){
				state = UP;
				on_time = get_tick_count() - on_time;
				led_on_time[0] = on_time;
				semaphore_give(&xSem);
			}
		}
	}
}

void* led_task(void* taskParam){
	while(1){
		semaphore_take(&xSem);
		gpioToggle(LED1);
		task_delay(led_on_time[0]);
		gpioToggle(LED1);
	}
}

#endif

#ifdef EJ1_bis
void* button_task(void* taskParam)
{
	uint32_t i;
	gpioMap_t tec;

	while(1){

		for(i=0; i< N_BUTTON ; i++)
		{
			tec = buttons_index[i];

			task_delay(10);
			if (!gpioRead(tec) && button_list[i].state == UP)
			{
				task_delay(20);
				if(!gpioRead(tec))
				{
					button_list[i].state = DOWN;
					button_list[i].begin_time = get_tick_count();
				}
			}

			if(gpioRead(tec) && button_list[i].state == DOWN)
			{
				task_delay(20);
				if(gpioRead(tec))
				{
					button_list[i].state = UP;
					button_list[i].end_time = get_tick_count();
					led_on_time[i] = button_list[i].end_time-button_list[i].begin_time;
					semaphore_give(&xSem);
				}
			}
		}
	}
}

void* led_task(void* taskParam){
	uint32_t led = LEDB;
	uint32_t i;

	while(1){
		semaphore_take(&xSem);

		for(i = 0; i < N_BUTTON ; i++)
		{
			if(led_on_time[i] > 0)
			{
				gpioToggle(led+i);
				task_delay(led_on_time[i]);
				gpioToggle(led+i);
				led_on_time[i] = 0;
				break;
			}
		}
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
	task_create(stack4,TASK_STACK_SIZE,task4,PRIORITY_LOW,(void *)0x22446688);
	#endif

	#ifdef EJ1
	semaphore_create(&xSem);
	task_create(stack1,TASK_STACK_SIZE,button_task,PRIORITY_LOW,(void *)0x11223344);
	task_create(stack2,TASK_STACK_SIZE,led_task,PRIORITY_LOW,(void *)0x55667788);
	#endif

	#ifdef EJ1_bis
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
