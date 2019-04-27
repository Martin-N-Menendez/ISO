#include "buttons.h"
#include "irq.h"

void tec_irq_enable(uint8_t irqChannel, uint8_t port, uint8_t pin, Buttons_State edge);
void Limpiar_Interrupcion(uint8_t irqChannel);
//void tec_irq_process(uint8_t irqChannel, uint8_t tec, TickType_t tickCount, BaseType_t xHigherPriorityTaskWoken);

void teclas_irq_enable(void)
{
	tec_irq_enable(0, 0, 4, UP); // TEC1 Presionado
	tec_irq_enable(1, 0, 4, DOWN); // TEC1 Soltado
	tec_irq_enable(2, 0, 8, UP); // TEC2 Presionado
	tec_irq_enable(3, 0, 8, DOWN); // TEC2 Soltado
}

void tec_irq_enable(uint8_t irqChannel, uint8_t port, uint8_t pin, Buttons_State edge)
{

	Chip_SCU_GPIOIntPinSel(irqChannel, port, pin);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(irqChannel));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(irqChannel));


	if (edge == UP)
	{
		Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(irqChannel));

	}
	if (edge == DOWN)
	{
		Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(irqChannel));
	}

}

void Limpiar_Interrupcion(uint8_t irqChannel)
{
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(irqChannel)); /* Clear interrupt flag for irqChannel */
}

void GPIO0_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = ( ( BaseType_t ) FALSE );

	if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH( 0 ) ) {

		tecla_presionada(1,UP);
		//printf( "up1 \r\n");

		Limpiar_Interrupcion(0);
	}

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GPIO1_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = ( ( BaseType_t ) FALSE );

	if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH( 1 ) ) {

		tecla_presionada(1,DOWN);
		//printf( "down1 \r\n");

		Limpiar_Interrupcion(1);
	}

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GPIO2_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = ( ( BaseType_t ) FALSE );

	if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH( 2 ) ) {

		tecla_presionada(2,UP);
		//printf( "up2 \r\n");

		Limpiar_Interrupcion(2);
	}

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GPIO3_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = ( ( BaseType_t ) FALSE );

	if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH( 3 ) ) {

		tecla_presionada(2,DOWN);
		//printf( "down2 \r\n");

		Limpiar_Interrupcion(3);
	}

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
