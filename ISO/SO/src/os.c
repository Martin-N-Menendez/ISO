/*==================[inclusions]=============================================*/
#include "board.h"
#include "os.h"
#include <strings.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
uint32_t tick_count = 0;
/*==================[internal functions declaration]=========================*/

#define FUNC0						0x0				/** Pin function 0 */
#define FUNC1						0x1				/** Pin function 1 */
#define FUNC2						0x2				/** Pin function 2 */
#define FUNC3						0x3				/** Pin function 3 */
#define FUNC4						0x4				/** Pin function 4 */
#define FUNC5						0x5				/** Pin function 5 */
#define FUNC6						0x6				/** Pin function 6 */
#define FUNC7						0x7				/** Pin function 7 */

#define OFF 						0x0
/*==================[internal data definition]===============================*/

task_struct task_list[N_TASK];
uint32_t current_task;
uint32_t task_list_idx;

uint32_t stack_idle[STACK_SIZE/4];

typedef struct{
   int8_t port;
   int8_t pin;
} pinConfigLpc4337_t;

typedef struct{
   int8_t port;
   int8_t pin;
} gpioConfigLpc4337_t;

typedef struct{
	pinConfigLpc4337_t pin_name;
	int8_t func;
	gpioConfigLpc4337_t gpio;
} pin_config_gpio_lpc_4337_t;

const pin_config_gpio_lpc_4337_t gpio_pins_config[] = {

   { {4, 1}, FUNC0, {2, 1} },   /*   0   CON1_36   T_FIL1           */
   { {7, 5}, FUNC0, {3,13} },   /*   1   CON1_34   T_COL2           */

   { {1, 5}, FUNC0, {1, 8} },   /*   2   CON1_39   T_COL0           */
   { {4, 2}, FUNC0, {2, 2} },   /*   3   CON1_37   T_FIL2           */
   { {4, 3}, FUNC0, {2, 3} },   /*   4   CON1_35   T_FIL3           */
   { {4, 0}, FUNC0, {2, 0} },   /*   5   CON1_33   T_FIL0           */
   { {7, 4}, FUNC0, {3,12} },   /*   6   CON1_31   T_COL1           */

   { {3, 2}, FUNC4, {5, 9} },   /*   7   CON1_29   CAN_TD           */
   { {3, 1}, FUNC4, {5, 8} },   /*   8   CON1_27   CAN_RD           */

   { {2, 3}, FUNC4, {5, 3} },   /*   9   CON1_25   RS232_TXD        */
   { {2, 4}, FUNC4, {5, 4} },   /*  10   CON1_23   RS232_RXD        */

   { {6,12}, FUNC0, {2, 8} },   /*  11   CON2_40   GPIO8            */
   { {6,11}, FUNC0, {3, 7} },   /*  12   CON2_38   GPIO7            */
   { {6, 9}, FUNC0, {3, 5} },   /*  13   CON2_36   GPIO5            */
   { {6, 7}, FUNC4, {5,15} },   /*  14   CON2_34   GPIO3            */
   { {6, 4}, FUNC0, {3, 3} },   /*  15   CON2_32   GPIO1            */

   { {4, 4}, FUNC0, {2, 4} },   /*  16   CON2_30   LCD1             */
   { {4, 5}, FUNC0, {2, 5} },   /*  17   CON2_28   LCD2             */
   { {4, 6}, FUNC0, {2, 6} },   /*  18   CON2_26   LCD3             */
   { {4, 8}, FUNC4, {5,12} },   /*  19   CON2_24   LCDRS            */
   { {4,10}, FUNC4, {5,14} },   /*  20   CON2_22   LCD4             */

   { {1, 3}, FUNC0, {0,10} },   /*  21   CON2_18   SPI_MISO         */

   { {1,20}, FUNC0, {0,15} },   /*  22   CON2_16   ENET_TXD1        */
   { {1,18}, FUNC0, {0,13} },   /*  23   CON2_14   ENET_TXD0        */
   { {1,17}, FUNC0, {0,12} },   /*  24   CON2_12   ENET_MDIO        */
   { {1,16}, FUNC0, {0, 3} },   /*  25   CON2_10   ENET_CRS_DV      */
   { {7, 7}, FUNC0, {3,15} },   /*  26   CON2_08   ENET_MDC         */
   { {0, 1}, FUNC0, {0, 1} },   /*  27   CON2_06   ENET_TXEN        */
   { {0, 0}, FUNC0, {0, 0} },   /*  28   CON2_04   ENET_RXD1        */

   { {6,10}, FUNC0, {3, 6} },   /*  29   CON2_35   GPIO6            */
   { {6, 8}, FUNC4, {5,16} },   /*  30   CON2_33   GPIO4            */
   { {6, 5}, FUNC0, {3, 4} },   /*  31   CON2_31   GPIO2            */
   { {6, 1}, FUNC0, {3, 0} },   /*  32   CON2_29   GPIO0            */

   { {4, 9}, FUNC4, {5,13} },   /*  33   CON2_23   LCDEN            */

   { {1, 4}, FUNC0, {0,11} },   /*  34   CON2_21   SPI_MOSI         */

   { {1,15}, FUNC0, {0, 2} },   /*  35   CON2_09   ENET_RXD0        */


   { {1, 0}, FUNC0, {0, 4} },   /* 36   TEC1    TEC_1                 */
   { {1, 1}, FUNC0, {0, 8} },   /* 37   TEC2    TEC_2                 */
   { {1, 2}, FUNC0, {0, 9} },   /* 38   TEC3    TEC_3                 */
   { {1, 6}, FUNC0, {1, 9} },   /* 39   TEC4    TEC_4                 */

   { {2, 0}, FUNC4, {5, 0} },   /* 43   LEDR    LED0_R                */
   { {2, 1}, FUNC4, {5, 1} },   /* 44   LEDG    LED0_G                */
   { {2, 2}, FUNC4, {5, 2} },   /* 45   LEDB    LED0_B                */
   { {2,10}, FUNC0, {0,14} },   /* 40   LED1    LED1                  */
   { {2,11}, FUNC0, {1,11} },   /* 41   LED2    LED2                  */
   { {2,12}, FUNC0, {1,12} },   /* 42   LED3    LED3                  */
};

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
	task_create(stack_idle,STACK_SIZE,idle, (void*)0);	/* Crear tarea Idle */
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

void task_create(uint32_t stack[],
				uint32_t stack_size_bytes,
				task_type entry_point,
				void * args){

	task_list[task_list_idx].id = task_list_idx;	/* Asignar indice de tarea */
	task_list[task_list_idx].state = RUNNING;		/* Asignar estado de tarea */
	/*Inicializar stack de tarea*/
	init_stack(stack,stack_size_bytes,&task_list[task_list_idx].stack_pointer,entry_point,args);

	task_list_idx++;
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

void task_delay(uint32_t delay)
{
	task_list[current_task].state = WAITING;				/* Asignar estado WAITING */
	task_list[current_task].wait_state = WAIT_TICKS;
	task_list[current_task].ticks = delay;					/* Guardar cantidad de ticks a esperar */
	while (task_list[current_task].ticks > 0){
		__WFI();											/* Esperar interrupciones mientras tanto */
	}
}

void task_return_hook(void* args)
{
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


/*==================[external functions definition]==========================*/

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

void led_set( uint8_t LEDNumber, bool On ){

	Board_LED_Set(LEDNumber-LEDR,On);
}


/*==================[end of file]============================================*/
