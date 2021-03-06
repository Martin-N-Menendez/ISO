/*==================[inclusions]=============================================*/
#include "board.h"
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include "os.h"
#include "sapi.h"
#include "task.h"
#include "semaphore.h"
#include "buttons.h"
#include "uart.h"
#include "irq.h"
/*==================[macros and definitions]=================================*/

#define TASK_FREQ 10

#define MSG_VERDE		"\r\n Led verde:"
#define MSG_ROJO		"\r\n Led rojo:"
#define MSG_AZUL		"\r\n Led azul:"
#define MSG_AMARILLO	"\r\n Led amarillo:"
#define ENCENDIDO		"\r\n\t t_on:"
#define ASCENDENTE		"\r\n\t t_asc:"
#define DESCENDENTE		"\r\n\t t_desc:"
#define SALTO			"\r\n"

TickType_t TiempoEncendido[N_BUTTON] = {0,0};
semaphore_t Msg,Led;
bool_t Publicado = FALSE;
TickType_t t1=0,t2=0;

gpioMap_t Botonera[] = { TEC1,TEC2 };

extern volatile Buttons_t button_list[N_BUTTON];

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

static void initHardware(void);

// Prototipo de funcion de la tarea
void* Antirebote( void* arg );
void* Maquina_de_Estados( void* arg );
void* Mensaje( void* arg );
void* LED_titilar( void* arg );
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
extern uint32_t stack4[12*TASK_STACK_SIZE/4];
extern uint32_t stack5[TASK_STACK_SIZE/4];
//extern uint32_t stack6[TASK_STACK_SIZE/4];

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
				if(button_list[BUTTON1].end_time > button_list[BUTTON2].end_time && Publicado == FALSE)
				{
					// T2 < 0 --> Rojo o Azul
					t2 = button_list[BUTTON1].end_time - button_list[BUTTON2].end_time;
					Tecla_Reiniciar_Final(); // Reiniciar tiempos para evitar volver a usarlos

					t2_state = 1;
					semaphore_give(&Msg);	// Semaforo para publicar el mensaje
					task_delay(TASK_FREQ);
				}
				if(button_list[BUTTON1].end_time < button_list[BUTTON2].end_time && Publicado == FALSE)
				{
					// T2 > 0 --> Verde o Amarillo
					t2 = button_list[BUTTON2].end_time - button_list[BUTTON1].end_time;
					Tecla_Reiniciar_Final(); // Reiniciar tiempos para evitar volver a usarlos

					t2_state = 0;
					semaphore_give(&Msg);	// Semaforo para publicar el mensaje
					task_delay(TASK_FREQ);
				}
			}
			if(button_list[BUTTON1].state == UP && button_list[BUTTON1].begin_time > 0 && button_list[1].begin_time > 0 ) // 0-1
			{
				// Presionando las teclas
				if(button_list[BUTTON2].begin_time > button_list[BUTTON1].begin_time && Publicado == FALSE)
				{
					// T1 > 0 --> Verde o Rojo
					t1 = button_list[BUTTON2].begin_time - button_list[BUTTON1].begin_time;
					Tecla_Reiniciar_Inicio(); // Reiniciar tiempos para evitar volver a usarlos

					t1_state = 0;
					task_delay(TASK_FREQ);
				}
				if(button_list[BUTTON2].begin_time < button_list[BUTTON1].begin_time && Publicado == FALSE)
				{
					// T1 < 0 --> Amarillo o Azul
					t1 = button_list[BUTTON1].begin_time - button_list[BUTTON2].begin_time;
					Tecla_Reiniciar_Inicio(); // Reiniciar tiempos para evitar volver a usarlos

					t1_state = 1;
					task_delay(TASK_FREQ);
				}

			}
		}

		// Si los estados son distintos, no han sido publicados aun
		if(button_list[BUTTON1].state != button_list[BUTTON2].state)
		{
			Publicado = FALSE;
		}

		// Si ambas teclas no son presionadas, reiniciar los tiempos de ambas teclas
		if(button_list[BUTTON1].state == DOWN && button_list[BUTTON2].state == DOWN)
		{
			Tecla_Reiniciar_Inicio();
			Tecla_Reiniciar_Final();
		}

		task_delay(TASK_FREQ);
	}

}

void* Antirebote( void* arg )
{
	// ---------- CONFIGURACIONES ------------------------------
	int i = 0;
	// ---------- REPETIR POR SIEMPRE --------------------------
	while(TRUE) {

			if (button_list[BUTTON1].state == DOWN && button_list[BUTTON2].state == DOWN)
			{
				TiempoEncendido[BUTTON1] = button_list[BUTTON1].end_time - button_list[BUTTON1].begin_time;
				TiempoEncendido[BUTTON2] = button_list[BUTTON2].end_time - button_list[BUTTON2].begin_time;

				if (Tick_Absoluto(TiempoEncendido[BUTTON1]) > ANTIREBOTE && Tick_Absoluto(TiempoEncendido[BUTTON2]) > ANTIREBOTE)
				{
					semaphore_give(&Led);
					task_delay(TASK_FREQ);
				}
				else // Si no pasan el antirebote, reiniciar tiempos
				{
					Tecla_Reiniciar_Inicio();
					Tecla_Reiniciar_Final();
					t1 = 0;
					t2 = 0;
				}
			}
			task_delay(TASK_FREQ);
	}
}

void* LED_titilar( void* arg )
{
	while(TRUE){
		semaphore_take(&Led);
		switch(color_state)
		{
			case VERDE:
				gpioWrite( LEDG , ON);
				task_delay( t1+t2 );
				gpioWrite( LEDG , OFF);
				break;
			case AMARILLO:
				gpioWrite( LED1 , ON);
				task_delay( t1+t2 );
				gpioWrite( LED1 , OFF);
				break;
			case ROJO:
				gpioWrite( LEDR , ON);
				task_delay( t1+t2 );
				gpioWrite( LEDR , OFF);
				break;
			case AZUL:
				gpioWrite( LEDB , ON);
				task_delay( t1+t2 );
				gpioWrite( LEDB , OFF);
				break;
			default:
				os_error_hook(1);
				break;
		}

	}
}

void* Mensaje( void* arg )
{
	while(TRUE){
		semaphore_take(&Msg); // Tomo el semaforo para el mensaje

		char t1_s[7];
		char t2_s[7];
		char t_s[7];

		itoa(t1,	t1_s,	10);
		itoa(t2,	t2_s,	10);
		itoa(t1+t2,	t_s,	10);

		switch(color_state)
		{
			case VERDE:
				UART_USB_String( MSG_VERDE );
				break;
			case AMARILLO:
				UART_USB_String( MSG_AMARILLO );
				break;
			case ROJO:
				UART_USB_String( MSG_ROJO );
				break;
			case AZUL:
				UART_USB_String( MSG_AZUL );
				break;
			default:
				os_error_hook(2);
				break;
		}

		UART_USB_String( ENCENDIDO );
		UART_USB_String( t_s );
		UART_USB_String( ASCENDENTE );
		UART_USB_String( t1_s );
		UART_USB_String( DESCENDENTE );
		UART_USB_String( t2_s );

		Publicado = TRUE; // Flag de que ya fue publicado
		task_delay(TASK_FREQ);
	}
}

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	NVIC_SetPriority(PendSV_IRQ, (1 << NVIC_PRIO_BITS) -1);

	//NVIC_SetPriority(-2, (1 << 3) -1);
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
	UART_USB_String( "[ISO]" );

	// Led Watchdog
	gpioWrite( LED3 , ON );

	// Crear tarea en freeRTOS

	task_create(stack1,TASK_STACK_SIZE,Antirebote,PRIORITY_LOW,(void *)0x11223344);
	task_create(stack2,TASK_STACK_SIZE,LED_titilar,PRIORITY_LOW,(void *)0x11223344);
	task_create(stack3,TASK_STACK_SIZE,Maquina_de_Estados,PRIORITY_LOW,(void *)0x11223344);
	task_create(stack4,12*TASK_STACK_SIZE,Mensaje,PRIORITY_LOW,(void *)0x11223344); // estas tareas tienen ALGO mal
	task_create(stack5,TASK_STACK_SIZE,FSM_color,PRIORITY_LOW,(void *)0x11223344);

	//Inicializo un semáforo binario para sincronizar la tecla con el led

	semaphore_create(&Msg);
	semaphore_create(&Led);

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
