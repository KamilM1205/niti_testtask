#ifndef _UPROTO_H_
#define _UPROTO_H_

#include "stm32f4xx_hal_def.h"
#include "list.h"

typedef struct UProto_t
{
  MyList_t*		rx_list;
  MyList_t*		tx_list;
  UART_HandleTypeDef	huart;
  uint8_t		buffer[9];
};

void	UProto_Init(UProto_t *up);
void	UProto_Receive(UProto_t *up);
void	UProto_Transmit(UProto_t *up);

__weak void UProto_ClbkReceive();
__weak void UProto_ClbkTransmit();

#endif	/* _UPROTO_H_*/
