	.syntax unified
	.text
	.global PendSV_Handler

	.extern get_next_context
	.extern add_tick_count

	.thumb_func

PendSV_Handler:
	/* Contexto FPU */
	tst			lr,0x10				/* LR & 0x10 (comparacion bit a bit) */
	it			eq					/* Si el resultado da igual que 0 */
	vpusheq		{s16-s31}			/* Push S16-S31 a la pila y fuerzo stacking S0-S15 */

	push 		{r4-r11,lr}			/* Push R4-R11 y el LR */
	bl 			add_tick_count
	mrs 		r0,msp 				/* r0 = msp*/
	bl 			get_next_context 	/* get_next_context(msp) */
	msr 		msp,r0				/* msp = r0 (valor devuelto por get_next_context) */

	pop {r4-r11,lr}

	tst			lr,0x10
	it			eq
	vpopeq		{s16-s31}
return:
	bx 			lr
