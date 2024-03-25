#ifndef _UPROTO_H_
#define _UPROTO_H_

#include "stm32f4xx_hal.h"
#include "ulist.h"

typedef void
(*callback_ptr) (void *up, char *data);

typedef struct
{
  uint8_t 	cmd[5];
  callback_ptr 	callback;
} UProtoCommand_t;

typedef struct
{
  MyList_t 		*rx_list;
  MyList_t 		*tx_list;
  UART_HandleTypeDef 	*huart;
  uint8_t 		buffer[10];
  UProtoCommand_t 	*callbacks;
  size_t 		callback_count;
  uint8_t 		tdata[9];
} UProto_t;

UProto_t*
UProto_Init (UART_HandleTypeDef *huart);
void
UProto_Receive (UProto_t *up);
void
UProto_Receive_IT (UProto_t *up, UART_HandleTypeDef *huart);
void
UProto_Transmit (UProto_t *up);
void
UProto_Transmit_IT (UProto_t *up, UART_HandleTypeDef *huart);
void
UProto_SendData (UProto_t *up, char data[5]);
void
UProto_AddCallback (UProto_t *up, char cmd[5], callback_ptr callback);

#endif	/* _UPROTO_H_*/
