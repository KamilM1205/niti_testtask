#ifndef _IBUS_H_
#define _IBUS_H_

#include "stm32f4xx_hal.h"
#include "ulist.h"

#define IBUS_PACKAGE_LEN	32
#define IBUS_JOY_MAX		2000
#define IBUS_JOY_MIDDLE		1500
#define IBUS_JOY_MIN		1000
#define IBUS_SWITCH_ON		0x07d0
#define IBUS_SWITCH_OFF		0x03e8
#define IBUS_VR_MAX		0x07d0
#define IBUS_VR_MIN		0x03e8

typedef void
(*callback_ptr) (void *ibus, uint16_t data);

typedef enum
{
  CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14
} IBUS_Channel;

typedef struct
{
  IBUS_Channel channel;
  callback_ptr callback;
} IBusCommand_t;

typedef struct
{
  MyList_t *rx_list;
  UART_HandleTypeDef *huart;
  uint8_t buffer[IBUS_PACKAGE_LEN + 1];
  IBusCommand_t *callbacks;
  size_t callback_count;
  uint8_t tdata[9];
} IBus_t;

IBus_t*
IBUS_Init (UART_HandleTypeDef *huart);
void
IBUS_Receive (IBus_t *up);
void
IBUS_Receive_IT (IBus_t *up, UART_HandleTypeDef *huart);
void
IBUS_Transmit (IBus_t *up);
void
IBUS_Transmit_IT (IBus_t *up, UART_HandleTypeDef *huart);
void
IBUS_SendData (IBus_t *up, char *data);
void
IBUS_AddCallback (IBus_t *up, IBUS_Channel channel, callback_ptr callback);

#endif	/* _IBUS_H_*/
