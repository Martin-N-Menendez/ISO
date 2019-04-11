#ifndef OS_H
#define OS_H

#include <stdint.h>
#include <stdlib.h>
#include "task.h"
#include "semaphore.h"

#define bool _Bool

//typedef enum{TEC1 = 36,  TEC2,  TEC3,  TEC4, LEDR,  LEDG,  LEDB,  LED1,  LED2,  LED3} gpioMap_t;

//typedef enum{
   /* EDU-CIAA-NXP */
/*
   // P1 header
   T_FIL1,    T_COL2,    T_COL0,    T_FIL2,      T_FIL3,  T_FIL0,     T_COL1,
   CAN_TD,    CAN_RD,    RS232_TXD, RS232_RXD,

   // P2 header
   GPIO8,     GPIO7,     GPIO5,     GPIO3,       GPIO1,
   LCD1,      LCD2,      LCD3,      LCDRS,       LCD4,
   SPI_MISO,
   ENET_TXD1, ENET_TXD0, ENET_MDIO, ENET_CRS_DV, ENET_MDC, ENET_TXEN, ENET_RXD1,
   GPIO6,     GPIO4,     GPIO2,     GPIO0,
   LCDEN,
   SPI_MOSI,
   ENET_RXD0,
*/
   // Switches
   // 36   37     38     39
  // TEC1=36,  TEC2,  TEC3,  TEC4,

   // Leds
   // 40   41     42     43     44     45
   //LEDR,  LEDG,  LEDB,  LED1,  LED2,  LED3,

   /* CIAA-NXP */
 /* 46     47     48     49     50     51     52     53 */
 //  DI0,   DI1,   DI2,   DI3,   DI4,   DI5,   DI6,   DI7,
 /* 54     55     56     57     58     59     60     61 */
 //  DO0,   DO1,   DO2,   DO3,   DO4,   DO5,   DO6,   DO7
//} gpioMap_t;


void os_init(void);
void schedule(void);
void SysTick_Handler(void);

void init_stack(uint32_t stack[],
				uint32_t stack_size_bytes,
				uint32_t * sp,
				task_type entry_point,
				void * arg);
uint32_t get_next_context(uint32_t current_sp);

void* idle(void* args);
void add_tick_count(void);
uint32_t get_tick_count(void);
void led_set( uint8_t LEDNumber, bool On );

#endif
