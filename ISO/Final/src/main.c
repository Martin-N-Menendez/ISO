/*==================[inclusions]=============================================*/
#include "board.h"
#include <stdlib.h>
#include <strings.h>
#include "os.h"
#include "sapi.h"
#include "task.h"
#include "semaphore.h"
#include "buttons.h"
#include "uart.h"
#include "irq.h"
/*==================[macros and definitions]=================================*/

#define TASK_FREQ 10

TickType_t TiempoEncendido[2] = {0,0};
semaphore_t S_1,S_2; // Semaforos para leds
semaphore_t M_01,M_10; // Semaforos para mensajes
semaphore_t Msg,Led;
bool_t Publicado_01 = FALSE;
bool_t Publicado_10 = FALSE;
TickType_t TiempoNoSolapado = 0;

gpioMap_t Botonera[] = { TEC1,TEC2 };

extern volatile Buttons_t button_list[N_BUTTON];

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void initHardware(void);

// Prototipo de funcion de la tarea
void* Antirebote( void* arg );
//void* LED1_titilar( void* arg );
//void* LED2_titilar( void* arg );
void* Maquina_de_Estados( void* arg );
//void* Mensaje_01( void* arg );
//void* Mensaje_10( void* arg );

void* Mensaje( void* arg );
void* Led_titilar( void* arg );
void* FSM_color( void* arg );

TickType_t Tick_Absoluto(TickType_t valor);

typedef enum {
	VERDE,
	AMARILLO,
	ROJO,
	AZUL
} Color_State;

extern void teclas_irq_enable(void);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

DEBUG_PRINT_ENABLE

extern uint32_t stack1[TASK_STACK_SIZE/4];
extern uint32_t stack2[TASK_STACK_SIZE/4];
extern uint32_t stack3[TASK_STACK_SIZE/4];
extern uint32_t stack4[TASK_STACK_SIZE/4];
extern uint32_t stack5[TASK_STACK_SIZE/4];
//extern uint32_t stack6[TASK_STACK_SIZE/4];

uint32_t led_on_time[N_BUTTON] = {0,0};
uint32_t current_task;

/*==================[internal functions definition]==========================*/

/*==================[definiciones de funciones internas]=====================*/

TickType_t Tick_Absoluto(TickType_t valor)
{
	if(valor < 0)
		return -valor;
	else
		return valor;
}

/*==================[definiciones de funciones externas]=====================*/

// Si t_b2 > t_b1 -> 0, si t_b2 < t_b1 -> 1

/*************	T1 = 0	|	T1 = 1
 *
 * T2 = 0	|	VERDE	|	AMARILLO
 *
 * T2 = 1	|	ROJO	|	AZUL
 *
 *  00 VERDE
 *  01 AMARILLO
 *  10 ROJO
 *  11 AZUL
 */

bool_t t1_state = 0;
bool_t t2_state = 0;
Color_State color_state = VERDE;

void* FSM_color( void* arg )
{
	while(1)
	{
		if (t1_state == 0)
			color_state = (t2_state)?(ROJO):(VERDE);
		if (t1_state == 1)
			color_state = (t2_state)?(AZUL):(AMARILLO);
		task_delay(TASK_FREQ);
	}
}

void* Maquina_de_Estados( void* arg )
{
	// Entiendo que no es lo mas "feliz" llamarla maquina de estados si no use un switch

	while(TRUE) { // ---------- REPETIR POR SIEMPRE --------------------------

		if(button_list[BUTTON1].state == button_list[BUTTON2].state) // Estados iguales -> mido las transiciones pasadas
		{
			if(button_list[BUTTON1].state == DOWN && button_list[BUTTON1].end_time > 0 && button_list[BUTTON2].end_time > 0) // 1-0
			{
				// Soltando las teclas
				if(button_list[BUTTON1].end_time > button_list[BUTTON2].end_time && Publicado_10 == FALSE)
				{
					// T2 < 0 --> Rojo o Azul
					TiempoNoSolapado = button_list[BUTTON1].end_time - button_list[BUTTON2].end_time;
					Tecla_Reiniciar_Final(); // Reiniciar tiempos para evitar volver a usarlos

					t2_state = 1;
					semaphore_give(&Msg);	// Semaforo para publicar el mensaje <10>
					//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
					task_delay(TASK_FREQ);
				}
				if(button_list[BUTTON1].end_time < button_list[BUTTON2].end_time && Publicado_01 == FALSE)
				{
					// T2 > 0 --> Verde o Amarillo
					TiempoNoSolapado = button_list[BUTTON2].end_time - button_list[BUTTON1].end_time;
					Tecla_Reiniciar_Final(); // Reiniciar tiempos para evitar volver a usarlos

					t2_state = 0;
					semaphore_give(&Msg);	// Semaforo para publicar el mensaje <01>
					//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
					task_delay(TASK_FREQ);
				}
				//vTaskDelay( TASK_FREQ / portTICK_RATE_MS );
			}
			if(button_list[BUTTON1].state == UP && button_list[BUTTON1].begin_time > 0 && button_list[1].begin_time > 0 ) // 0-1
			{
				// Presionando las teclas
				if(button_list[BUTTON2].begin_time > button_list[BUTTON1].begin_time && Publicado_01 == FALSE)
				{
					// T1 > 0 --> Verde o Rojo
					TiempoNoSolapado = button_list[BUTTON2].begin_time - button_list[BUTTON1].begin_time;
					Tecla_Reiniciar_Inicio(); // Reiniciar tiempos para evitar volver a usarlos

					t1_state = 0;
					//semaphore_give(&Msg);	// Semaforo para publicar el mensaje <01>
					//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
					task_delay(TASK_FREQ);
				}
				if(button_list[BUTTON2].begin_time < button_list[BUTTON1].begin_time && Publicado_10 == FALSE)
				{
					// T1 < 0 --> Amarillo o Azul
					TiempoNoSolapado = button_list[BUTTON1].begin_time - button_list[BUTTON2].begin_time;
					Tecla_Reiniciar_Inicio(); // Reiniciar tiempos para evitar volver a usarlos

					t1_state = 1;
					//semaphore_give(&Msg);	// Semaforo para publicar el mensaje <10>
					//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
					task_delay(TASK_FREQ);
				}

			}
		}

		// Si los estados son distintos, no han sido publicados aun
		if(button_list[BUTTON1].state != button_list[BUTTON2].state)
		{
			Publicado_01 = FALSE;
			Publicado_10 = FALSE;
		}

		// Si ambas teclas no son presionadas, reiniciar los tiempos de ambas teclas
		if(button_list[BUTTON1].state == DOWN && button_list[BUTTON2].state == DOWN)
		{
			Tecla_Reiniciar_Inicio();
			Tecla_Reiniciar_Final();
		}

		//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
		task_delay(TASK_FREQ);
	}

}

void* Antirebote( void* arg )
{
	// ---------- CONFIGURACIONES ------------------------------
	int i = 0;
	// ---------- REPETIR POR SIEMPRE --------------------------
	while(TRUE) {

		for( i = 0 ; i< N_BUTTON ; i++ )
		{
			if (button_list[i].state == UP)
			{
				TiempoEncendido[i] = button_list[i].end_time - button_list[i].begin_time;

				//if (Tick_Absoluto(TiempoEncendido[i]) > ANTIREBOTE / portTICK_RATE_MS)
				if (Tick_Absoluto(TiempoEncendido[i]) > ANTIREBOTE )
				{
					(i==0)?(semaphore_give(&Led)):(semaphore_give(&Led)); // Libero el semaforo para el led
					//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
					task_delay(TASK_FREQ);
				}
				else // Si no pasan el antirebote, reiniciar tiempos
				{
					button_list[i].begin_time = 0;
					button_list[i].end_time = 0;
				}
			}
			//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
			task_delay(TASK_FREQ);
		}
	}
}

/*
void* LED1_titilar( void* arg )
{
	while(1){
		//xSemaphoreTake(S_1,portMAX_DELAY);
		semaphore_take(&S_1);
		gpioWrite( LEDB , ON);
		//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
		task_delay(TASK_FREQ);
		gpioWrite( LEDB , OFF);
		//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
		task_delay(TASK_FREQ);
	}
}

void* LED2_titilar( void* arg )
{
	while(TRUE){
		//xSemaphoreTake(S_2,portMAX_DELAY);
		semaphore_take(&S_2);
		gpioWrite( LED1 , ON);
		//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
		task_delay(TASK_FREQ);
		gpioWrite( LED1 , OFF);
		//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
		task_delay(TASK_FREQ);
	}
}

void* Mensaje_10( void* arg )
{
	while(TRUE){
		//xSemaphoreTake(M_10,portMAX_DELAY); // Tomo el semaforo para el mensaje
		semaphore_take(&M_10); // Tomo el semaforo para el mensaje
		//printf( "{1:0:%u} \r\n", TiempoNoSolapado / portTICK_RATE_MS); // Publico mensaje en formato pedido, con ms
		//printf( "{1:0:%u} \r\n", (unsigned int) TiempoNoSolapado ); // Publico mensaje en formato pedido, con ms
		debugPrintlnString( "10" );
		Publicado_10 = TRUE; // Flag de que ya fue publicado
		//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
		task_delay(TASK_FREQ);
	}
}

void* Mensaje_01( void* arg )
{
	while(TRUE){
		//xSemaphoreTake(M_01,portMAX_DELAY); // Tomo el semaforo para el mensaje
		semaphore_take(&M_01); // Tomo el semaforo para el mensaje
		//printf( "{0:1:%u} \r\n", TiempoNoSolapado / portTICK_RATE_MS); // Publico mensaje en formato pedido, con ms
		//printf( "{0:1:%u} \r\n", (unsigned int) TiempoNoSolapado ); // Publico mensaje en formato pedido, con ms
		debugPrintlnString( "01" );
		Publicado_01 = TRUE; // Flag de que ya fue publicado
		//vTaskDelay(	TASK_FREQ / portTICK_RATE_MS );
		task_delay(TASK_FREQ);
	}
}
*/

void* LED_titilar( void* arg )
{
	while(TRUE){
		semaphore_take(&S_2);
		gpioWrite( LED1 , ON);
		task_delay(TASK_FREQ);
		gpioWrite( LED1 , OFF);
		task_delay(TASK_FREQ);
	}
}

void* Mensaje( void* arg )
{
	while(TRUE){
		semaphore_take(&Msg); // Tomo el semaforo para el mensaje
		//printf( "{1:0:%u} \r\n", TiempoNoSolapado / portTICK_RATE_MS); // Publico mensaje en formato pedido, con ms
		//printf( "{1:0:%u} \r\n", (unsigned int) TiempoNoSolapado ); // Publico mensaje en formato pedido, con ms
		switch(color_state)
		{
		case VERDE:
			debugPrintlnString( "VERDE" );
			break;
		case AMARILLO:
			debugPrintlnString( "AMARILLO" );
			break;
		case ROJO:
			debugPrintlnString( "ROJO" );
			break;
		case AZUL:
			debugPrintlnString( "AZUL" );
			break;
		default: break;
		}

		Publicado_10 = TRUE; // Flag de que ya fue publicado
		task_delay(TASK_FREQ);
	}
}

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	//NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) -1);

	NVIC_SetPriority(-2, (1 << 3) -1);
	SysTick_Config(SystemCoreClock / 1000);
}

/*==================[external functions definition]==========================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{
	// ---------- CONFIGURACIONES ------------------------------
	// Inicializar y configurar la plataforma
	initHardware(); 						/* Inicializar la placa */
	boardConfig();
	buttons_begin();
	teclas_irq_enable();

	NVIC_ClearPendingIRQ( PIN_INT0_IRQ );	//Borra el clear pending de la IRQ y lo activa
	//Establezco la prioridad de la interrupción
	NVIC_SetPriority( PIN_INT0_IRQ, 7); //Debe ser mayor (número) a configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
	NVIC_EnableIRQ( PIN_INT0_IRQ );

	NVIC_ClearPendingIRQ( PIN_INT1_IRQ ); //Borra el clear pending de la IRQ y lo activa
	//Establezco la prioridad de la interrupción
	NVIC_SetPriority(PIN_INT1_IRQ, 7); //Debe ser mayor (número) a configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
	NVIC_EnableIRQ( PIN_INT1_IRQ );

	NVIC_ClearPendingIRQ( PIN_INT2_IRQ ); //Borra el clear pending de la IRQ y lo activa
	//Establezco la prioridad de la interrupción
	NVIC_SetPriority(PIN_INT2_IRQ, 7); //Debe ser mayor (número) a configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
	NVIC_EnableIRQ( PIN_INT2_IRQ );

	NVIC_ClearPendingIRQ( PIN_INT3_IRQ ); //Borra el clear pending de la IRQ y lo activa
	//Establezco la prioridad de la interrupción
	NVIC_SetPriority(PIN_INT3_IRQ, 7); //Debe ser mayor (número) a configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
	NVIC_EnableIRQ( PIN_INT3_IRQ );

	// UART for debug messages
	debugPrintConfigUart( UART_USB, 115200 );
	debugPrintlnString( "Prueba" );

	//UART_config( UART_USB , BAUD_RATE );
	//UART_USB_String( MSG_WELCOME );
	//UART_print_string( MSG_WELCOME );

	//float_to_string(314.15);

	// Led Watchdog
	gpioWrite( LED3 , ON );

	// Crear tarea en freeRTOS

	task_create(stack1,TASK_STACK_SIZE,Antirebote,PRIORITY_LOW,(void *)0x11223344);
	task_create(stack2,TASK_STACK_SIZE,LED_titilar,PRIORITY_LOW,(void *)0x11223344);
	task_create(stack3,TASK_STACK_SIZE,Maquina_de_Estados,PRIORITY_LOW,(void *)0x11223344);
	task_create(stack4,TASK_STACK_SIZE,Mensaje,PRIORITY_LOW,(void *)0x11223344); // estas tareas tienen ALGO mal
	task_create(stack5,TASK_STACK_SIZE,FSM_color,PRIORITY_LOW,(void *)0x11223344);

	//Inicializo un semáforo binario para sincronizar la tecla con el led

	//semaphore_create(&S_1);
	//semaphore_create(&S_2);
	//semaphore_create(&M_01);
	//semaphore_create(&M_10);

	semaphore_create(&Msg);
	semaphore_create(&Led);

	/*
	if ( NULL == S_1 ){
		while (1);
	}

	if ( NULL == S_2 ){
		while (1);
	}

	if ( NULL == M_01 ){
		while (1);
	}

	if ( NULL == M_10 ){
		while (1);
	}
	*/

	// Iniciar scheduler
	os_init();

	// ---------- REPETIR POR SIEMPRE --------------------------
	while( TRUE ) {
		// Si cae en este while 1 significa que no pudo iniciar el scheduler
		gpioToggle(LEDR);
		__WFI();
	}

	// NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
	// directamenteno sobre un microcontroladore y no es llamado por ningun
	// Sistema Operativo, como en el caso de un programa para PC.
	return 0;
}

/*==================[end of file]============================================*/
