/*
 * Backup_driver.c
 *
 *  Created on: 14.04.2017
 *      Author: Adam Miarka
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/

/*-------------------------------- INCLUDE DIRECTIVES ----------------------------------*/
#include "Backup_driver.h"

/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/

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

void Backup_Init(void)
{
	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Enable the PWR clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);

	/* Allow access to RTC */
	PWR_BackupAccessCmd(ENABLE);

	/* Wait until the Backup SRAM low power Regulator is ready */
	while(PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET)
	{
	}
}

void Backup_Deinit(void)
{
	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, DISABLE);

	/* Disable the PWR clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, DISABLE);

	/* Disable the Backup SRAM low power Regulator */
	PWR_BackupRegulatorCmd(DISABLE);
}

BackupStatus_T Backup_Set_Value(uint32_t offset, uint32_t data)
{
	BackupStatus_T ret = BACKUP_STATUS_FAILED;

	if((offset % 4) == 0)
	{
		(*(__IO uint32_t *) (BKPSRAM_BASE + offset)) = data;

		ret =  BACKUP_STATUS_SUCCESS;
	}
	else
	{
		ret = BACKUP_STATUS_WRONG_OFFSET;
	}

	return ret;
}

BackupStatus_T Backup_Reset_Value(uint32_t offset)
{
	BackupStatus_T ret = BACKUP_STATUS_FAILED;

	if((offset % 4) == 0)
	{
		*(__IO uint32_t *) (BKPSRAM_BASE + offset) = (uint32_t) 0x0;

		ret = BACKUP_STATUS_SUCCESS;
	}
	else
	{
		ret = BACKUP_STATUS_WRONG_OFFSET;
	}

	return ret;
}

BackupStatus_T Backup_Check_Value(uint32_t offset, uint32_t value)
{
	BackupStatus_T ret = BACKUP_STATUS_FAILED;

	if((offset % 4) == 0)
	{
		if(*(__IO uint32_t *) (BKPSRAM_BASE + offset) == value )
		{
			ret =  BACKUP_STATUS_SUCCESS;
		}
		else
		{
			ret = BACKUP_STATUS_FAILED;
		}
	}
	else
	{
		ret = BACKUP_STATUS_WRONG_OFFSET;
	}

	return ret;
}


/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/

/**
 * @}
 */






