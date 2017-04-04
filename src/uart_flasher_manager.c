/*=======================================================================================*
 * @file    uart_flasher_manager.c
 * @author  Damian Pala, Adam Miarka
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
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "diag/Trace.h"

#include "uart_flasher_manager.h"
#include "uart_flasher_driver.h"

/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/
#define SEND_TIMEOUT                          100000000 // TODO calculate for current clk freq
#define WRITE_DATA_BLOCK_SIZE                 256
#define PACKET_TO_SEND_MAX_SIZE               WRITE_DATA_BLOCK_SIZE + DEVICE_NUM_OF_BYTES_TO_WRITE_SIZE + DEVICE_CHECKSUM_SIZE
#define DEVICE_RSP_ACK                        0x79
#define DEVICE_RSP_NACK                       0x1F
#define DEVICE_START_FLASHING                 0x7F
#define DEVICE_WRITE_MEMORY_CMD_H             0x31
#define DEVICE_WRITE_MEMORY_CMD_L             0xCE
#define DEVICE_EXTENDED_ERASE_CMD_H           0x44
#define DEVICE_EXTENDED_ERASE_CMD_L           0xBB
#define DEVICE_MASS_ERASE_CMD_H               0xFF
#define DEVICE_MASS_ERASE_CMD_L               0xFF
#define DEVICE_READOUT_PROTECT_CMD_H		  0x82
#define DEVICE_READOUT_PROTECT_CMD_L		  0x7D
#define DEVICE_READOUT_UNPROTECT_CMD_H		  0x92
#define DEVICE_READOUT_UNPROTECT_CMD_L		  0x6D
#define DEVICE_CODE_MEMORY_START_ADDR         0x08000000
#define DEVICE_MIN_BYTES_TO_MEM_WRITE         4
#define DEVICE_NUM_OF_BYTES_TO_WRITE_SIZE     1
#define DEVICE_CHECKSUM_SIZE                  1
#define SEND_DATA_MAX_TRIALS                  3

/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/
#define SWAP_UINT32(x)                        (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/
typedef enum Status_Tag
{
  STATUS_SUCCESS = 1,
  STATUS_FAILED
} Status_T;

typedef enum SendData_Tag
{
  SEND_DATA_RX_ACK = 1,
  SEND_DATA_RX_NACK,
  SEND_DATA_TIMEOUT,
  SEND_DATA_ERROR,
} SendData_T;

typedef enum WriteMemoryState_Tag
{
  WRITE_MEM_SEND_WRITE_CMD = 1,
  WRITE_MEM_SEND_ADDRESS,
  WRITE_MEM_SEND_DATA,
  WRITE_MEM_EXIT,
} WriteMemoryState_T;

/*------------------------------- STRUCT AND UNIONS ------------------------------------*/
typedef struct PacketToSend_Tag
{
  uint8_t packetData[PACKET_TO_SEND_MAX_SIZE];
  size_t packetSize;
} PacketToSend_T;

/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/
static volatile bool ReceivedDataReady = false;
PacketToSend_T PacketToSend;

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/
SendData_T SendData(uint8_t *data, size_t size);
void PreparePacketWithChecksum(uint8_t * const data, size_t dataSize);
void PrepareMemDataPacket(uint8_t * const data, size_t dataSize);
Status_T StartFlashingProcedure(void);
Status_T MassErase(void);
Status_T PageErase(uint16_t startPageNumber, uint16_t numOfPagesToErase);
Status_T WriteMemory(uint32_t address, uint8_t *data, size_t size);
Status_T ReadoutProtect(void);
Status_T ReadoutUnprotect(void);
// tutaj napisac funkcje do blokowania flasha

/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/
void UFM_DataReceivedNotification(void)
{
  ReceivedDataReady = true;
}

#define FLASH_DEVICE_MAX_TRIALS         3

UFM_FlashingStatus_T UFM_FlashDevice(uint8_t *data, size_t size)
{
  UFM_FlashingStatus_T ret = FLASHING_STATUS_FAILED;
  Status_T stepRet = STATUS_FAILED;
  size_t numOfFullPackets = 0;
  bool isPartialPacketToSend = false;
  uint32_t offset = 0;
  uint8_t trialCnt = 0;

  numOfFullPackets = size/WRITE_DATA_BLOCK_SIZE;

  if ( (size % WRITE_DATA_BLOCK_SIZE) != 0)
  {
    isPartialPacketToSend = true;
  }
  else
  {
    isPartialPacketToSend = false;
  }

  trace_printf("Number of full packets to write: %d\n", numOfFullPackets);
  trace_printf("Number of bytes to write in partial packet: %d\n", size % WRITE_DATA_BLOCK_SIZE);

  while ( (trialCnt < FLASH_DEVICE_MAX_TRIALS) && (ret != FLASHING_STATUS_SUCCESS) )
  {
    trialCnt++;
    offset = 0;

    stepRet = StartFlashingProcedure();

    if (STATUS_SUCCESS == stepRet)
    {
      trace_puts("Flashing starting success");
      stepRet = MassErase();
    }
    else
    {
      trace_puts("Flashing starting error");
      ret = FLASHING_STATUS_FAILED;
    }

    if (STATUS_SUCCESS == stepRet)
    {
      trace_puts("Mass erase success");
      for (uint16_t packetCnt = 0; packetCnt < numOfFullPackets; packetCnt++)
      {
        stepRet = WriteMemory(DEVICE_CODE_MEMORY_START_ADDR + offset, data + offset, WRITE_DATA_BLOCK_SIZE);
        offset += WRITE_DATA_BLOCK_SIZE;

        if (stepRet != STATUS_SUCCESS)
        {
          break;
        }
      }

      if (isPartialPacketToSend && (STATUS_SUCCESS == stepRet))
      {
        stepRet = WriteMemory(DEVICE_CODE_MEMORY_START_ADDR + offset, data + offset, size % WRITE_DATA_BLOCK_SIZE);
      }
    }
    else
    {
      trace_puts("Mass erase error");
      ret = FLASHING_STATUS_FAILED;
    }

    if (STATUS_SUCCESS == stepRet)
    {
      trace_puts("Flashing packets success");
      ret = FLASHING_STATUS_SUCCESS;
    }
    else
    {
      trace_puts("Flashing packets error");
      ret = FLASHING_STATUS_FAILED;
    }
  }

  return ret;
}

/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/
Status_T StartFlashingProcedure(void)
{
  uint8_t byteToSend = DEVICE_START_FLASHING;
  SendData_T sendStatus = SEND_DATA_ERROR;

  sendStatus = SendData(&byteToSend, sizeof(byteToSend));

  switch (sendStatus)
  {
    case SEND_DATA_RX_ACK:
    {
      return STATUS_SUCCESS;

      break;
    }
    case SEND_DATA_RX_NACK:
    case SEND_DATA_TIMEOUT:
    case SEND_DATA_ERROR:
    default:
    {
      return STATUS_FAILED;

      break;
    }
  }

  return 0;
}

Status_T MassErase(void)
{
  Status_T ret = STATUS_FAILED;
  SendData_T sendStatus = SEND_DATA_ERROR;
  uint8_t cmdToSend[2];

  cmdToSend[0] = DEVICE_EXTENDED_ERASE_CMD_H;
  cmdToSend[1] = DEVICE_EXTENDED_ERASE_CMD_L;

  sendStatus = SendData(cmdToSend, sizeof(cmdToSend));

  if (SEND_DATA_RX_ACK == sendStatus)
  {
    cmdToSend[0] = DEVICE_MASS_ERASE_CMD_H;
    cmdToSend[1] = DEVICE_MASS_ERASE_CMD_L;

    PreparePacketWithChecksum(cmdToSend, sizeof(cmdToSend));
    sendStatus = SendData(PacketToSend.packetData, PacketToSend.packetSize);

    switch (sendStatus)
    {
      case SEND_DATA_RX_ACK:
      {
        ret = STATUS_SUCCESS;

        break;
      }
      case SEND_DATA_RX_NACK:
      case SEND_DATA_TIMEOUT:
      case SEND_DATA_ERROR:
      default:
      {
        ret = STATUS_FAILED;

        break;
      }
    }
  }
  else
  {
    ret = STATUS_FAILED;
  }

  return ret;
}

//Status_T PageErase(uint16_t startPageNumber, uint16_t numOfPagesToErase)
//{
//  // TODO: may be implement in future
////  Status_T ret = STATUS_FAILED;
////  SendData_T sendStatus = SEND_DATA_ERROR;
////  uint8_t cmdToSend[2];
////
////  cmdToSend[0] = DEVICE_EXTENDED_ERASE_CMD_H;
////  cmdToSend[1] = DEVICE_EXTENDED_ERASE_CMD_L;
////
////  sendStatus = SendData(cmdToSend, sizeof(cmdToSend));
////
////  if (SEND_DATA_RX_ACK == sendStatus)
////  {
////    cmdToSend[0] = DEVICE_MASS_ERASE_CMD_H;
////    cmdToSend[1] = DEVICE_MASS_ERASE_CMD_L;
////
////    PreparePacketWithChecksum(cmdToSend, sizeof(cmdToSend));
////    sendStatus = SendData(PacketToSend.packetData, PacketToSend.packetSize);
////
////    switch (sendStatus)
////    {
////      case SEND_DATA_RX_ACK:
////      {
////        ret = STATUS_SUCCESS;
////
////        break;
////      }
////      case SEND_DATA_RX_NACK:
////      case SEND_DATA_TIMEOUT:
////      case SEND_DATA_ERROR:
////      default:
////      {
////        ret = STATUS_FAILED;
////
////        break;
////      }
////    }
////  }
////  else
////  {
////    ret = STATUS_FAILED;
////  }
////
////  return ret;
//}

Status_T WriteMemory(uint32_t address, uint8_t *data, size_t size)
{
  Status_T ret = STATUS_FAILED;
  SendData_T sendStatus = SEND_DATA_ERROR;
  uint8_t cmdToSend[2];
  WriteMemoryState_T writeMemoryState = WRITE_MEM_SEND_WRITE_CMD;
  uint8_t loopCnt = 10;

  while( (loopCnt > 0) && (writeMemoryState != WRITE_MEM_EXIT) )
  {
    loopCnt--;

    switch (writeMemoryState)
    {
      case WRITE_MEM_SEND_WRITE_CMD:
      {
        cmdToSend[0] = DEVICE_WRITE_MEMORY_CMD_H;
        cmdToSend[1] = DEVICE_WRITE_MEMORY_CMD_L;

        sendStatus = SendData(cmdToSend, sizeof(cmdToSend));

        if (SEND_DATA_RX_ACK == sendStatus)
        {
          writeMemoryState = WRITE_MEM_SEND_ADDRESS;
        }
        else
        {
          ret = STATUS_FAILED;
          writeMemoryState = WRITE_MEM_EXIT;
        }

        break;
      }
      case WRITE_MEM_SEND_ADDRESS:
      {
        uint32_t addressSwapped = SWAP_UINT32(address); // Swap address bytes because bootloader needs big endian order while ARM has little endian architecture
        PreparePacketWithChecksum((uint8_t*)&addressSwapped, sizeof(addressSwapped));
        sendStatus = SendData(PacketToSend.packetData, PacketToSend.packetSize);

        if (SEND_DATA_RX_ACK == sendStatus)
        {
          writeMemoryState = WRITE_MEM_SEND_DATA;
        }
        else
        {
          ret = STATUS_FAILED;
          writeMemoryState = WRITE_MEM_EXIT;
        }

        break;
      }
      case WRITE_MEM_SEND_DATA:
      {
        PrepareMemDataPacket(data, size);
        sendStatus = SendData(PacketToSend.packetData, PacketToSend.packetSize);

        if (SEND_DATA_RX_ACK == sendStatus)
        {
          ret = STATUS_SUCCESS;
          writeMemoryState = WRITE_MEM_EXIT;
        }
        else
        {
          ret = STATUS_FAILED;
          writeMemoryState = WRITE_MEM_EXIT;
        }

        break;
      }
      case WRITE_MEM_EXIT:
      default:
      {
        /* Do nothing */
        break;
      }
    }
  }

  return ret;
}

void PreparePacketWithChecksum(uint8_t * const data, size_t dataSize)
{
  uint8_t checksum = 0;

  memcpy((void*)PacketToSend.packetData, (void*)data, dataSize);

  for (size_t byteCnt = 0; byteCnt < dataSize; byteCnt++)
  {
    checksum ^= PacketToSend.packetData[byteCnt];
  }

  PacketToSend.packetData[dataSize] = checksum;
  PacketToSend.packetSize = dataSize + DEVICE_CHECKSUM_SIZE;
}

void PrepareMemDataPacket(uint8_t * const data, size_t dataSize)
{
  uint8_t checksum = 0;
  uint16_t numOfBytesToWrite = 0;

  if ((dataSize % DEVICE_MIN_BYTES_TO_MEM_WRITE) != 0)
  {
    numOfBytesToWrite = (dataSize/DEVICE_MIN_BYTES_TO_MEM_WRITE + 1)*DEVICE_MIN_BYTES_TO_MEM_WRITE;
  }
  else
  {
    numOfBytesToWrite = dataSize;
  }

  memset((void*)PacketToSend.packetData, 0xFF, numOfBytesToWrite + DEVICE_NUM_OF_BYTES_TO_WRITE_SIZE);
  PacketToSend.packetData[0] = numOfBytesToWrite - 1; // - 1 according to USART protocol used in the STM32 bootloader document
  memcpy((void*)(&PacketToSend.packetData[1]), (void*)data, dataSize);

  for (uint16_t byteCnt = 0; byteCnt < numOfBytesToWrite + DEVICE_NUM_OF_BYTES_TO_WRITE_SIZE; byteCnt++)
  {
    checksum ^= PacketToSend.packetData[byteCnt];
  }

  PacketToSend.packetData[numOfBytesToWrite + DEVICE_NUM_OF_BYTES_TO_WRITE_SIZE] = checksum;
  PacketToSend.packetSize = numOfBytesToWrite + DEVICE_NUM_OF_BYTES_TO_WRITE_SIZE + DEVICE_CHECKSUM_SIZE;
}

typedef enum SendDataState_Tag
{
  SEND_DATA_STATE_SEND = 1,
  SEND_DATA_STATE_CHECK_RSP,
  SEND_DATA_TRY_AGAIN,
  SEND_DATA_EXIT,
} SendDataState_T;

SendData_T SendData(uint8_t *data, size_t size)
{
  SendData_T ret = SEND_DATA_ERROR;
  SendDataState_T sendDataState = SEND_DATA_STATE_SEND;
  uint8_t loopCnt = 10;
  uint32_t timeout = SEND_TIMEOUT;
  uint8_t trialCnt = 0;

  while( (loopCnt > 0) && (sendDataState != SEND_DATA_EXIT) )
  {
    loopCnt--;

    switch (sendDataState)
    {
      case SEND_DATA_STATE_SEND:
      {
        for (size_t byteCnt = 0; byteCnt < size; byteCnt++)
        {
          UFD_SendByte(data[byteCnt]);
        }

        ReceivedDataReady = false;

        while( (timeout > 0) && (false == ReceivedDataReady) )
        {
          timeout--;
        }

        if (0 == timeout)
        {
          trace_puts("Send data timeout");
          ret = SEND_DATA_TIMEOUT;
          sendDataState = SEND_DATA_TRY_AGAIN;
        }
        else
        {
          sendDataState = SEND_DATA_STATE_CHECK_RSP;
        }

        break;
      }
      case SEND_DATA_STATE_CHECK_RSP:
      {
        uint8_t receivedResponse = 0;

        UFD_ReceiveByte(&receivedResponse);

        if (DEVICE_RSP_ACK == receivedResponse)
        {
          ret = SEND_DATA_RX_ACK;
        }
        else if (DEVICE_RSP_NACK == receivedResponse)
        {
          ret = SEND_DATA_RX_NACK;
          sendDataState = SEND_DATA_TRY_AGAIN;
        }
        else
        {
          ret = SEND_DATA_ERROR;
          sendDataState = SEND_DATA_TRY_AGAIN;
        }

        break;
      }
      case SEND_DATA_TRY_AGAIN:
      {
        if (trialCnt < SEND_DATA_MAX_TRIALS)
        {
          trialCnt++;
          sendDataState = SEND_DATA_STATE_SEND;
        }
        else
        {
          sendDataState = SEND_DATA_EXIT;
        }

        break;
      }
      case SEND_DATA_EXIT:
      default:
      {
        /* Do nothing */
        break;
      }
    }
  }

  return ret;
}

Status_T ReadoutProtect(void)
{
	Status_T ret = STATUS_FAILED;
	SendData_T sendStatus = SEND_DATA_ERROR;
	uint8_t cmdToSend[2];

	cmdToSend[0] = DEVICE_READOUT_PROTECT_CMD_H;
	cmdToSend[1] = DEVICE_READOUT_PROTECT_CMD_L;

	sendStatus = SendData(cmdToSend, sizeof(cmdToSend));

		if (SEND_DATA_RX_ACK == sendStatus)
		{
			/* TODO: check this */
			sendStatus = SendData(NULL,0);

		    switch (sendStatus)
		    {
		      case SEND_DATA_RX_ACK:
		      {
		        ret = STATUS_SUCCESS;

		        break;
		      }
		      case SEND_DATA_RX_NACK:
		      case SEND_DATA_TIMEOUT:
		      case SEND_DATA_ERROR:
		      default:
		      {
		        ret = STATUS_FAILED;
		        break;
		      }
		    }
		}
	return ret;
}


Status_T ReadoutUnprotect(void)
{
	Status_T ret = STATUS_FAILED;
	SendData_T sendStatus = SEND_DATA_ERROR;
	uint8_t cmdToSend[2];

	cmdToSend[0] = DEVICE_READOUT_PROTECT_CMD_H;
	cmdToSend[1] = DEVICE_READOUT_PROTECT_CMD_L;

	sendStatus = SendData(cmdToSend, sizeof(cmdToSend));


}



/**
 * @}
 */
