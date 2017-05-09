/*
 * Boot_jumper.c
 *
 *  Created on: 19.04.2017
 *      Author: Adam Miarka
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/

/*-------------------------------- INCLUDE DIRECTIVES ----------------------------------*/
#include "Boot_jumper.h"
#include "Backup_driver.h"
/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/
#define BOOT_BKP_VAL		(uint32_t)0x1
#define BOOT_BKP_REG_NR		RTC_BKP_DR0
#define BOOTLOADER_ADDRESS	0x1FFF0000
#define APPLICATION_ADDRESS	0x1FFF0004
#define RAM_VAl				0x20002E48
/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
void (*JumpToBootloader)(void);

/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/
typedef enum Status_Tag
{
	STATUS_FAILED = 0,
	STATUS_SUCCESS,
	STATUS_WRONG_OFFSET
} Status_T;

/*------------------------------- STRUCT AND UNIONS ------------------------------------*/

/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/
void Boot_Init(void);
void Boot_Deinit(void);
/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/

BootJumperStatus_T Boot_StartupHandler(void)
{
	Status_T stepRet = STATUS_FAILED;
	BootJumperStatus_T ret = BOOT_JUMPER_STATUS_FAILED;

	JumpToBootloader = (void(*)(void))(*((uint32_t *) APPLICATION_ADDRESS));

	Boot_Init();

	if(RTC_ReadBackupRegister(BOOT_BKP_REG_NR) == BOOT_BKP_VAL)
	{
		stepRet = STATUS_SUCCESS;
	}

	if(stepRet == STATUS_SUCCESS)
	{
		RTC_WriteBackupRegister(BOOT_BKP_REG_NR, 0);

		RCC_DeInit();

		Boot_Deinit();

		__set_MSP(RAM_VAl);

		JumpToBootloader();

		ret = BOOT_JUMPER_STATUS_SUCCESS;
	}
	else
	{
		ret = BOOT_JUMPER_STATUS_FAILED;
	}

	return ret;
}

BootJumperStatus_T Boot_JumpToBootloaderAfterReset(void)
{
	BootJumperStatus_T ret = BOOT_JUMPER_STATUS_FAILED;

	/* Write to the RTC Backup Data Register */
	RTC_WriteBackupRegister(BOOT_BKP_REG_NR, BOOT_BKP_VAL);

	NVIC_SystemReset();

	ret = BOOT_JUMPER_STATUS_SUCCESS;

	return ret;

}
/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/
void Boot_Init(void)
{
	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC */
	PWR_BackupAccessCmd(ENABLE);

	/* Enable the LSI OSC */
	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{
	}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

	/* Enable the RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC APB registers synchronization */
	RTC_WaitForSynchro();
}


void Boot_Deinit(void)
{
	/* Disable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, DISABLE);

	/* Forbid access to RTC */
	PWR_BackupAccessCmd(DISABLE);

	/* Disable the LSI OSC */
	RCC_LSICmd(DISABLE);

	/* Disable the RTC Clock */
	RCC_RTCCLKCmd(DISABLE);
}

/**
 * @}
 */






