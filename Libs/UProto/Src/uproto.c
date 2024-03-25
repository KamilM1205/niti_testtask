#include "uproto.h"
#include "cmsis_os.h"
#include <string.h>

void
UProto_Error_Handler ()
{
  __disable_irq ();

  while (1)
    {
    }
}

void
UProto_Init (UProto_t *uproto)
{
  uproto = (UProto_t*) pvPortMalloc (sizeof(UProto_t));
  memset (uproto, 0, sizeof(UProto_t));

  uproto->huart.Instance = USART1;
  uproto->huart.Init.BaudRate = 115200;
  uproto->huart.Init.WordLength = UART_WORDLENGTH_8B;
  uproto->huart.Init.StopBits = UART_STOPBITS_1;
  uproto->huart.Init.Parity = UART_PARITY_NONE;
  uproto->huart.Init.Mode = UART_MODE_TX_RX;
  uproto->huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  uproto->huart.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init (&uproto->huart) != HAL_OK)
    {
      UProto_Error_Handler ();
    }

  __HAL_RCC_DMA2_CLK_ENABLE();

  HAL_NVIC_SetPriority (DMA2_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ (DMA2_Stream7_IRQn);
}

uint8_t
CountCRC8 (uint8_t data[5])
{
  uint8_t crc = 0xFF;

  for (size_t i = 0; i < 5; i++)
    {
      crc ^= data[i];

      for (size_t j = 0; j < 8; j++)
	{
	  if ((crc & 0x80) != 0)
	    {
	      crc = (uint8_t) ((crc << 1) ^ 0x31);
	    }
	  else
	    {
	      crc <<= 1;
	    }
	}
    }

  return crc;
}

void
UProto_FindSignature (UProto_t *up, uint8_t buff[9])
{
  if (buff[0] == 0x01 && buff[1] == 0x02 && buff[8] == 0x5A)
    {
      if (CountCRC8(&buff[2]) != buff[7])
	{
	  return;
	}

      uint8_t data[5];
      memcpy (&data, &buff[2], 5);
      MyListPushBack (up->rx_list, (uint8_t*)&data);
    }
}

void
UProto_Receive (UProto_t *up)
{
  if (up->rb_count > 0)
    {
      for (size_t i = 0; i < up->rb_count; i++)
	{
	  memmove (&up->buffer, &up->buffer[2], 8);
	  up->buffer[8] = up->rb[i];
	  UProto_FindSignature (up, up->buffer);
	}
      up->rb_count = 0;
    }

  if (HAL_UART_GetState (&up->huart) != HAL_UART_STATE_BUSY_RX)
    {
      HAL_UART_Receive_IT (&up->huart, &up->rb[up->rb_count], 1);
      up->rb_count++;
    }

  if (!MyListIsEmpty (up->rx_list))
    {
      MyListItem_t *item = MyListPopFront (up->rx_list);

      for (size_t i = 0; i < up->callback_count; i++)
	{
	  if (memcmp (&item->data, &up->callbacks[i].cmd, 5) == 0)
	    {
	      up->callbacks[i].callback (up, (char*) &item->data);
	    }
	}

      vPortFree (item);
    }
}

void
UProto_SendData (UProto_t *up, char data[5])
{
  MyListPushBack (up->tx_list, (uint8_t*) &data);
  UProto_Transmit (up);
}

void
UProto_Receive_IT (UProto_t *up, UART_HandleTypeDef *huart)
{
  if (huart == &up->huart
      && HAL_UART_GetState (huart) != HAL_UART_STATE_BUSY_RX)
    {
      HAL_UART_Receive_IT (huart, &up->rb[up->rb_count], 1);
      up->rb_count++;
    }
}

void
UProto_Transmit_IT (UProto_t *up, UART_HandleTypeDef *huart)
{
  if (huart == &up->huart)
    {
      UProto_Transmit (up);
    }
}

void
UProto_Transmit (UProto_t *up)
{
  if (HAL_UART_GetState (&up->huart) != HAL_UART_STATE_BUSY_TX
      && !MyListIsEmpty (up->tx_list))
    {
      uint8_t tdata[9];
      MyListItem_t *item = MyListPopFront (up->tx_list);

      tdata[0] = 0x01;
      tdata[1] = 0x02;
      memcpy (&tdata[2], &item->data, 5);
      tdata[8] = CountCRC8 (&tdata[2]);
      tdata[9] = 0x5A;

      vPortFree (item);
      HAL_UART_Transmit_DMA (&up->huart, tdata, 9);
    }
}

void
UProto_AddCallback (UProto_t *up, char cmd[5], callback_ptr cb)
{
  UProtoCommand_t c =
    { .cmd = (uint8_t) cmd[0], .callback = cb };
  UProtoCommand_t *cmds = (UProtoCommand_t*) pvPortMalloc (
      sizeof(UProtoCommand_t) * up->callback_count);
  memcpy (&cmds, up->callbacks, sizeof(UProtoCommand_t) * up->callback_count);
  cmds[up->callback_count - 1] = c;
  vPortFree (up->callbacks);
  up->callbacks = cmds;
}
