/*=======================================================================================*
 * @file    Flasher
 * @author  Adam Miarka
 * @version 0.1
 * @date    17-01-2016
 * @brief   This file contains state machine dedicated to Smart Socket project
 *======================================================================================*/

/*----------------------- DEFINE TO PREVENT RECURSIVE INCLUSION ------------------------*/
#ifndef FLASHER_H_
#define FLASHER_H_

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/

/*---------------------- INCLUDE DIRECTIVES FOR STANDARD HEADERS -----------------------*/

/*----------------------- INCLUDE DIRECTIVES FOR OTHER HEADERS -------------------------*/
#include "cmsis_device.h"
#include "cortexm/ExceptionHandlers.h"
#include "main.h"
#include "USART.h"

/*-------------------------- EXPORTED DEFINES FOR CONSTANTS ----------------------------*/
#define   APB1_FREQ        42000000                           // Clock driving TIM3
#define   CNT_FREQ         21000000                           // TIM3 counter clock (prescaled APB1)
#define   IT_PER_SEC       2000                               // Interrupts per second
#define   TIM3_PULSE       ((CNT_FREQ) / (IT_PER_SEC))        // Output compare reg value
#define   TIM_PRESCALER    (((APB1_FREQ) / (CNT_FREQ))-1)     // APB1 prescaler

// Bootloader commands
#define ACK  (uint8_t)0x79
#define NACK (uint8_t)0x1F
/*------------------------------ EXPORTED DEFINE MACROS --------------------------------*/

/*======================================================================================*/
/*                     ####### EXPORTED TYPE DECLARATIONS #######                       */
/*======================================================================================*/

/* ------------------------------------ ENUMS ----------------------------------------- */

/*------------------------------------ STRUCT ------------------------------------------*/

/*------------------------------------ UNIONS ------------------------------------------*/

/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/
extern volatile uint8_t timeout;
extern volatile uint32_t fcount;
/*======================================================================================*/
/*                    ####### EXPORTED OBJECT DECLARATIONS #######                      */
/*======================================================================================*/
void FlasherConfig(void);
void Timer3Start(void);
void Timer3Stop(void);

uint8_t StartProcedure(void);
uint8_t WriteMemory(uint32_t indexStart, uint32_t indexStop);
uint8_t StartFlashing(uint8_t *flash,uint32_t NumberOfBytes);
uint8_t GlobalErase(void);

#endif /* Flasher_HPP_ */
