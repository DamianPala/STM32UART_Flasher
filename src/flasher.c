/*
 * flasher.c
 *
 *  Created on: 31.01.2017
 *      Author: Adam
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/
/*---------------------- INCLUDE DIRECTIVES FOR STANDARD HEADERS -----------------------*/
#include <flasher.h>
#include "cortexm/ExceptionHandlers.h"
#include "stm32f4xx.h"
#include "diag/Trace.h"
#include "string.h"
#include "USART.h"
/*----------------------- INCLUDE DIRECTIVES FOR OTHER HEADERS -------------------------*/

/*--------------------------- LOCAL DEFINES FOR CONSTANTS ------------------------------*/

/*------------------------------- LOCAL DEFINE MACROS ----------------------------------*/

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
/* ------------------------------------ ENUMS ----------------------------------------- */

/*------------------------------------ STRUCT ------------------------------------------*/

/*------------------------------------ UNIONS ------------------------------------------*/

/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*------------------------------- EXPORTED OBJECTS -------------------------------------*/

/*-------------------------------- LOCAL OBJECTS ---------------------------------------*/

volatile uint8_t timeout = 0;
volatile uint32_t fcount = 0;
uint8_t StartAddress[4] = {0x00, 0x00, 0x00, 0x8};
uint8_t checksum = 0;
uint8_t *flash = 0;

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/

/*======================================================================================*/
/*          ####### LOCAL INLINE FUNCTIONS AND FUNCTION-LIKE MACROS #######             */
/*======================================================================================*/

/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/
void FlasherConfig(void)
{
    TIM_TimeBaseInitTypeDef TIM3_TimeBase;
    TIM_OCInitTypeDef TIM3_OC;
    NVIC_InitTypeDef NVIC_TIM3;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    NVIC_TIM3.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_TIM3.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_TIM3.NVIC_IRQChannelSubPriority = 0;
    NVIC_TIM3.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_TIM3);

    TIM3_TimeBase.TIM_ClockDivision = 0;
    TIM3_TimeBase.TIM_CounterMode = TIM_CounterMode_Up;
    TIM3_TimeBase.TIM_Period = 0xFFFF;
    TIM3_TimeBase.TIM_Prescaler = TIM_PRESCALER * 2;
    TIM_TimeBaseInit(TIM3, &TIM3_TimeBase);

    TIM3_OC.TIM_OCMode = TIM_OCMode_Toggle;
    TIM3_OC.TIM_OutputState = TIM_OutputState_Enable;
    TIM3_OC.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM3_OC.TIM_Pulse = TIM3_PULSE;
    TIM_OC1Init(TIM3,&TIM3_OC);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);          // Disabling Ch.1 Output Compare preload

    TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);                     // Enabling TIM3 Ch.1 interrupts
}

void Timer3Start(void)
{
    TIM_Cmd(TIM3, ENABLE);                                      // Ready, Set, Go!
}

void Timer3Stop(void)
{
    TIM_Cmd(TIM3, DISABLE);                                      // Ready, Set, Stop!
}

uint8_t StartProcedure(void)
{
    ClearBuff();

    Send_u8b(0x7F);
    timeout = 0;
    Timer3Start();
    while(timeout<1)
    {
        if(RxBuff[Bufferpointer] == NACK)
        {
            Bufferpointer++;
            Timer3Stop();
            return 3;
        }
        if(RxBuff[Bufferpointer] == ACK)
        {
            Bufferpointer++;
            Timer3Stop();
            return 1;
        }

    }

    trace_puts("Timeout");
    Timer3Stop();
    return 2;
}

uint8_t WriteMemory(uint32_t indexStart, uint32_t indexStop)
{
    uint32_t index = 0;

    Send_u8b(0x31);
    Send_u8b(0xCE);

    timeout = 0;
    Timer3Start();
    while(timeout<1)
    {
        if(RxBuff[Bufferpointer] == NACK)
        {
            Bufferpointer++;
            Timer3Stop();
            return 3;
        }
        if(RxBuff[Bufferpointer] == ACK)
        {
            Bufferpointer++;
            Timer3Stop();
            timeout = 0;
            break;
        }

    }
    if(timeout>1)
    {
        Timer3Stop();
        trace_puts("Timeout");
        return 2;
    }

    checksum = StartAddress[3]^StartAddress[2]^StartAddress[1]^StartAddress[0];

    Send_u8b(StartAddress[3]);
    Send_u8b(StartAddress[2]);
    Send_u8b(StartAddress[1]);
    Send_u8b(StartAddress[0]);

    Send_u8b(checksum);

    timeout = 0;
    Timer3Start();
    while(timeout<1)
    {
        if(RxBuff[Bufferpointer] == NACK)
        {
            Bufferpointer++;
            Timer3Stop();
            return 3;
        }
        if(RxBuff[Bufferpointer] == ACK)
        {
            Bufferpointer++;
            Timer3Stop();
            timeout = 0;
            break;
        }
    }
    if(timeout>1)
    {
        Timer3Stop();
        trace_puts("Timeout");
        return 2;
    }

    if(((indexStop-indexStart)%4) == 0)
    {
    checksum = indexStop-indexStart-1;
    Send_u8b(indexStop-indexStart-1);
    }
    else if(((indexStop-indexStart)%4) == 1)
    {
        checksum = indexStop-indexStart+2;
        Send_u8b(indexStop-indexStart+2);
    }
    else if(((indexStop-indexStart)%4) == 2)
    {
        checksum = indexStop-indexStart+1;
        Send_u8b(indexStop-indexStart+1);
    }
    else if(((indexStop-indexStart)%4) == 3)
    {
        checksum = indexStop-indexStart;
        Send_u8b(indexStop-indexStart);
    }

    for(index = indexStart; index < indexStop; index++)
    {
        checksum = checksum^flash[index];
        Send_u8b(flash[index]);
    }

if(((indexStop-indexStart)%4) != 0)
{
        index = 0;
        while(index < (4-((indexStop- indexStart)%4)))
        {
            checksum = checksum^0xFF;
            Send_u8b(0xFF);
            index++;
        }
}

    Send_u8b(checksum);

    timeout = 0;
    Timer3Start();
    while(timeout<1)
    {
        if(RxBuff[Bufferpointer] == NACK)
        {
            Bufferpointer++;
            Timer3Stop();
            return 3;
        }
        if(RxBuff[Bufferpointer] == ACK)
        {
            Bufferpointer++;
            Timer3Stop();
            timeout = 0;
            break;
        }
    }
    if(timeout>1)
    {
        Timer3Stop();
        trace_puts("Timeout");
        return 2;
    }

    return 1;
}

uint8_t GlobalErase(void)
{


    Send_u8b(0x44);
    Send_u8b(0xBB);

    timeout = 0;
    Timer3Start();
    while(timeout<1)
    {
        if(RxBuff[Bufferpointer] == NACK)
        {
            Bufferpointer++;
            Timer3Stop();
            return 3;
        }
        if(RxBuff[Bufferpointer] == ACK)
        {
            Bufferpointer++;
            Timer3Stop();
            timeout = 0;
            break;
        }

    }
    if(timeout>1)
    {
        trace_puts("Timeout");
        return 2;
    }

    Send_u8b(0xFF);
    Send_u8b(0xFF);

    checksum = 0xFF^0xFF;

    Send_u8b(checksum);

    timeout = 0;
    Timer3Start();
    while(timeout<1)
    {
        if(RxBuff[Bufferpointer] == NACK)
        {
            Bufferpointer++;
            Timer3Stop();
            return 3;
        }
        if(RxBuff[Bufferpointer] == ACK)
        {
            Bufferpointer++;
            Timer3Stop();
            timeout = 0;
            break;
        }

    }
    if(timeout>1)
    {
        trace_puts("Timeout");
        return 2;
    }

    return 2;
}

uint8_t StartFlashing(uint8_t *bin, uint32_t NumberOfBytes)
{
    uint32_t NumberOfPackets = 0;
    uint8_t Rest = 0;
    uint32_t i = 0;

    flash = bin;
    uint32_t *PointeronStartAddress = (uint32_t*)StartAddress;

    NumberOfPackets = NumberOfBytes/128;
    Rest = (uint8_t)(NumberOfBytes % (uint32_t)128);

    StartProcedure();
    GlobalErase();

    for(i = 0; i< NumberOfPackets; i++)
    {
        WriteMemory(i*128, i*128+128);
        *PointeronStartAddress += 0x80;
    }


    if(Rest != 0)
    {
    WriteMemory(i*128, i*128+Rest);
    }


    return 1;
}
