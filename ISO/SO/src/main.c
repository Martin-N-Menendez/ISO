/*==================[inclusions]=============================================*/
#include "board.h"
#include <stdlib.h>
#include <strings.h>
#include "os.h"
#include "task.h"
#include "semaphore.h"
/*==================[macros and definitions]=================================*/

#define EJ1
#define DELAY_MS 1000
#define LED 2	//#define LED LEDR

typedef enum {UP,DOWN} button_state;
//typedef enum {TEC1,TEC2,TEC3,TEC4} button_idx;
//typedef enum {LEDR,LEDG,LEDB,LED1,LED2,LED3} led_idx;
typedef enum {OFF,ON} led_state;

semaphore_t xSem;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void initHardware(void);

#ifdef EJ0
static void * task1(void * param);
static void * task2(void * param);
static void * task3(void * param);
#endif

extern bool gpioRead( gpioMap_t pin );

/*==================[internal data definition]===============================*/

//static uint32_t pausems_count;

/*==================[external data definition]===============================*/

uint32_t stack1[STACK_SIZE/4];
uint32_t stack2[STACK_SIZE/4];
uint32_t stack3[STACK_SIZE/4];

uint32_t led_on_time_tick;
uint32_t launched = 0;

uint32_t current_task;

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) -1);
}

#ifdef EJ0
void * task1(void* arg)
{
	while(1){
		Board_LED_Toggle(LED1);
		task_delay(200);
	}
	return NULL;
}

void * task2(void* arg)
{
	while(1){
		Board_LED_Toggle(LED2);
		task_delay(300);
	}
	return NULL;
}

void * task3(void* arg)
{
	while(1){
		Board_LED_Toggle(LED3);
		task_delay(400);
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

		if(!gpioRead(36) && state == UP){
			task_delay(20);
			if(!gpioRead(36)){
				state = DOWN;
				on_time = get_tick_count();
			}
		}

		if(gpioRead(36) && state == DOWN){
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
		led_set(LEDR,ON);
		task_delay(led_on_time_tick);
		//Board_LED_Set(LED,OFF);
		led_set(LEDR,OFF);
	}
}

#endif

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){

	/* ------------- INICIALIZACIONES ------------- */

	os_init();

	#ifdef EJ0
	task_create(stack1,STACK_SIZE,task1,1,(void *)0x11223344);
	task_create(stack2,STACK_SIZE,task2,1,(void *)0x55667788);
	task_create(stack3,STACK_SIZE,task3,1,(void *)0x22446688);
	#endif

	#ifdef EJ1
	semaphore_create(&xSem);
	task_create(stack1,STACK_SIZE,button_task,1,(void *)0x11223344);
	task_create(stack2,STACK_SIZE,led_task,1,(void *)0x55667788);
	#endif


	initHardware(); /* Inicializar la placa */

	while(1) {  /* ------------- REPETIR POR SIEMPRE ------------- */
		//Board_LED_Toggle(LEDB);
		Board_LED_Set(LED+3,ON);
		//task_delay(500);
		//Board_LED_Set(LED,OFF);
		__WFI();
	}

	/* NO DEBE LLEGAR NUNCA AQUI*/
	return 0 ;
}

/*==================[end of file]============================================*/
