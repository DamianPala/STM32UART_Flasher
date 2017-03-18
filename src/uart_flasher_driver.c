/*=======================================================================================*
 * @file    uart_flasher_driver.c
 * @author  Damian Pala
 * @date    18-03-2017
 * @brief   This file contains all implementations for XXX module.
 *======================================================================================*/

/**
 * @addtogroup XXX Description
 * @{
 * @brief Module for... .
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/
/*-------------------------------- INCLUDE DIRECTIVES ----------------------------------*/
#include "cmsis_device.h"

#include "uart_flasher_driver.h"
#include "uart_flasher_manager.h"

/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/
#define BAUDRATE                          57600
#define RXBUFFSIZE 128

/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/

/*------------------------------- STRUCT AND UNIONS ------------------------------------*/

/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/

/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/
void UFD_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;

  // Enable clock for GPIOA
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  /**
  * Tell pins PA9 and PA10 which alternating function you will use
  * @important Make sure, these lines are before pins configuration!
  */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
  // Initialize pins as alternating function
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**
   * Enable clock for USART1 peripheral
   */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  /**
   * Set Baudrate to value you pass to function
   * Disable Hardware Flow control
   * Set Mode To TX and RX, so USART will work in full-duplex mode
   * Disable parity bit
   * Set 1 stop bit
   * Set Data bits to 8
   *
   * Initialize USART1
   * Activate USART1
   */
  USART_InitStruct.USART_BaudRate = BAUDRATE;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_InitStruct.USART_Parity = USART_Parity_Even;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_WordLength = USART_WordLength_9b;
  USART_Init(USART1, &USART_InitStruct);
  USART_Cmd(USART1, ENABLE);

  /**
   * Enable RX interrupt
   */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  /**
   * Set Channel to USART1
   * Set Channel Cmd to enable. That will enable USART1 channel in NVIC
   * Set Both priorities to 0. This means high priority
   *
   * Initialize NVIC
   */
  NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStruct);
}

void UFD_SendByte(uint8_t byte)
{
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // Wait for Empty
  USART_SendData(USART1, (uint16_t)byte);
}

void UFD_ReceiveByte(uint8_t *buffer)
{
    *buffer = (uint8_t)USART_ReceiveData(USART1);
}

void USART1_IRQHandler(void)
{
  if( USART_GetITStatus(USART1, USART_IT_RXNE) )
  {
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);

    UFM_DataReceivedNotification();
  }
}

/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/

/**
 * @}
 */
