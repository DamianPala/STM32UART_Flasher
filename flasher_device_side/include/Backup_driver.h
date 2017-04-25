/*=======================================================================================*
 * @file    Backup_driver.h
 * @author  Adam Miarka
 * @version 0.1
 * @date    17-01-2016
 * @brief   This file contains API functions allowing to handle backup
 *======================================================================================*/


/*
 * Note:
 * This library uses one of RTC backup registers.
 * */


/*----------------------- DEFINE TO PREVENT RECURSIVE INCLUSION ------------------------*/
#ifndef BACKUP_DRIVER_H_
#define BACKUP_DRIVER_H_

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/

/*---------------------- INCLUDE DIRECTIVES FOR STANDARD HEADERS -----------------------*/

/*----------------------- INCLUDE DIRECTIVES FOR OTHER HEADERS -------------------------*/
#include "cmsis_device.h"
#include "cortexm/ExceptionHandlers.h"
#include "main.h"

/*-------------------------- EXPORTED DEFINES FOR CONSTANTS ----------------------------*/

/*------------------------------ EXPORTED DEFINE MACROS --------------------------------*/

/*======================================================================================*/
/*                     ####### EXPORTED TYPE DECLARATIONS #######                       */
/*======================================================================================*/

/* ------------------------------------ ENUMS ----------------------------------------- */
typedef enum BackupStatus_Tag
{
	BACKUP_STATUS_FAILED = 0,
	BACKUP_STATUS_SUCCESS,
	BACKUP_STATUS_WRONG_OFFSET

} BackupStatus_T;
/*------------------------------------ STRUCT ------------------------------------------*/

/*------------------------------------ UNIONS ------------------------------------------*/

/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*======================================================================================*/
/*                    ####### EXPORTED OBJECT DECLARATIONS #######                      */
/*======================================================================================*/

/*======================================================================================*/
/*                   ####### EXPORTED FUNCTIONS PROTOTYPES #######                      */
/*======================================================================================*/

void Backup_Init(void);
void Backup_Deinit(void);
BackupStatus_T Backup_Set_Value(uint32_t offset, uint32_t data);
BackupStatus_T Backup_Reset_Value(uint32_t offset);
BackupStatus_T Backup_Check_Value(uint32_t offset, uint32_t value);

/*======================================================================================*/
/*                          ####### INLINE FUNCTIONS #######                            */
/*======================================================================================*/

#endif /* BACKUP_DRIVER_H_*/
