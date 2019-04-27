#ifndef IRQ_H_
#define IRQ_H_

typedef long BaseType_t;
typedef uint32_t TickType_t;

void GPIO0_IRQHandler(void);
void GPIO1_IRQHandler(void);
void GPIO2_IRQHandler(void);
void GPIO3_IRQHandler(void);

void tec_irq_enable(uint8_t irqChannel, uint8_t port, uint8_t pin, Buttons_State edge);
void teclas_irq_enable(void);
void Limpiar_Interrupcion(uint8_t irqChannel);


typedef enum {
	/* -------------------------  Cortex-M4 Processor Exceptions Numbers  ----------------------------- */
	Reset_IRQ                        = -15,/*!<   1  Reset Vector, invoked on Power up and warm reset */
	NonMaskableInt_IRQ               = -14,/*!<   2  Non maskable Interrupt, cannot be stopped or preempted */
	HardFault_IRQ                    = -13,/*!<   3  Hard Fault, all classes of Fault */
	MemoryManagement_IRQ             = -12,/*!<   4  Memory Management, MPU mismatch, including Access Violation and No Match */
	BusFault_IRQ                     = -11,/*!<   5  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory related Fault */
	UsageFault_IRQ                   = -10,/*!<   6  Usage Fault, i.e. Undef Instruction, Illegal State Transition */
	SVCall_IRQ                       =  -5,/*!<  11  System Service Call via SVC instruction */
	DebugMonitor_IRQ                 =  -4,/*!<  12  Debug Monitor                    */
	PendSV_IRQ                       =  -2,/*!<  14  Pendable request for system service */
	SysTick_IRQ                      =  -1,/*!<  15  System Tick Timer                */

	/* ---------------------------  LPC18xx/43xx Specific Interrupt Numbers  ------------------------------- */
	DAC_IRQ                          =   0,/*!<   0  DAC                              */
	M0APP_IRQ                        =   1,/*!<   1  M0APP Core interrupt             */
	DMA_IRQ                          =   2,/*!<   2  DMA                              */
	RESERVED1_IRQ                    =   3,/*!<   3  EZH/EDM                          */
	RESERVED2_IRQ                    =   4,
	ETHERNET_IRQ                     =   5,/*!<   5  ETHERNET                         */
	SDIO_IRQ                         =   6,/*!<   6  SDIO                             */
	LCD_IRQ                          =   7,/*!<   7  LCD                              */
	USB0_IRQ                         =   8,/*!<   8  USB0                             */
	USB1_IRQ                         =   9,/*!<   9  USB1                             */
	SCT_IRQ                          =  10,/*!<  10  SCT                              */
	RITIMER_IRQ                      =  11,/*!<  11  RITIMER                          */
	TIMER0_IRQ                       =  12,/*!<  12  TIMER0                           */
	TIMER1_IRQ                       =  13,/*!<  13  TIMER1                           */
	TIMER2_IRQ                       =  14,/*!<  14  TIMER2                           */
	TIMER3_IRQ                       =  15,/*!<  15  TIMER3                           */
	MCPWM_IRQ                        =  16,/*!<  16  MCPWM                            */
	ADC0_IRQ                         =  17,/*!<  17  ADC0                             */
	I2C0_IRQ                         =  18,/*!<  18  I2C0                             */
	I2C1_IRQ                         =  19,/*!<  19  I2C1                             */
	SPI_INT_IRQ                      =  20,/*!<  20  SPI_INT                          */
	ADC1_IRQ                         =  21,/*!<  21  ADC1                             */
	SSP0_IRQ                         =  22,/*!<  22  SSP0                             */
	SSP1_IRQ                         =  23,/*!<  23  SSP1                             */
	USART0_IRQ                       =  24,/*!<  24  USART0                           */
	UART1_IRQ                        =  25,/*!<  25  UART1                            */
	USART2_IRQ                       =  26,/*!<  26  USART2                           */
	USART3_IRQ                       =  27,/*!<  27  USART3                           */
	I2S0_IRQ                         =  28,/*!<  28  I2S0                             */
	I2S1_IRQ                         =  29,/*!<  29  I2S1                             */
	RESERVED4_IRQ                    =  30,
	SGPIO_INT_IRQ                    =  31,/*!<  31  SGPIO_IINT                       */
	PIN_INT0_IRQ                     =  32,/*!<  32  PIN_INT0                         */
	PIN_INT1_IRQ                     =  33,/*!<  33  PIN_INT1                         */
	PIN_INT2_IRQ                     =  34,/*!<  34  PIN_INT2                         */
	PIN_INT3_IRQ                     =  35,/*!<  35  PIN_INT3                         */
	PIN_INT4_IRQ                     =  36,/*!<  36  PIN_INT4                         */
	PIN_INT5_IRQ                     =  37,/*!<  37  PIN_INT5                         */
	PIN_INT6_IRQ                     =  38,/*!<  38  PIN_INT6                         */
	PIN_INT7_IRQ                     =  39,/*!<  39  PIN_INT7                         */
	GINT0_IRQ                        =  40,/*!<  40  GINT0                            */
	GINT1_IRQ                        =  41,/*!<  41  GINT1                            */
	EVENTROUTER_IRQ                  =  42,/*!<  42  EVENTROUTER                      */
	C_CAN1_IRQ                       =  43,/*!<  43  C_CAN1                           */
	RESERVED6_IRQ                    =  44,
	ADCHS_IRQ                        =  45,/*!<  45  ADCHS interrupt                  */
	ATIMER_IRQ                       =  46,/*!<  46  ATIMER                           */
	RTC_IRQ                          =  47,/*!<  47  RTC                              */
	RESERVED8_IRQ                    =  48,
	WWDT_IRQ                         =  49,/*!<  49  WWDT                             */
	M0SUB_IRQ                        =  50,/*!<  50  M0SUB core interrupt             */
	C_CAN0_IRQ                       =  51,/*!<  51  C_CAN0                           */
	QEI_IRQ                          =  52,/*!<  52  QEI                              */
} LPC43XX_IRQ_Type;

/* Scheduler utilities. */
extern void vPortYield( void );
#define portNVIC_INT_CTRL_REG		( * ( ( volatile unsigned long * ) 0xe000ed04 ) )
#define portNVIC_PENDSVSET_BIT		( 1UL << 28UL )
#define portYIELD()					vPortYield()
#define portEND_SWITCHING_ISR( xSwitchRequired ) if( xSwitchRequired ) portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT
#define portYIELD_FROM_ISR( x ) portEND_SWITCHING_ISR( x )

#endif
