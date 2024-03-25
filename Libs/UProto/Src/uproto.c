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

UProto_t*
UProto_Init (UART_HandleTypeDef *huart)
{
  UProto_t *uproto = (UProto_t*) pvPortMalloc (sizeof(UProto_t));
  memset (uproto, 0, sizeof(UProto_t));

  uproto->callbacks = pvPortMalloc (sizeof(UProtoCommand_t));
  uproto->rx_list = MyListInit();
  uproto->tx_list = MyListInit();
  uproto->huart = huart;

  return uproto;
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
      memset(&up->buffer, 0, 10);
    }
}

void
UProto_Receive (UProto_t *up)
{
  UProto_FindSignature (up, up->buffer);
//  if (up->rb_count > 0)
//    {
//      for (size_t i = 0; i < up->rb_count; i++)
//	{
//	  memmove (&up->buffer, &up->buffer[1], 8);
//	  up->buffer[8] = up->rb[i];
//	}
//      up->rb_count = 0;
//    }

  if (HAL_UART_GetState (up->huart) != HAL_UART_STATE_BUSY_RX)
    {
      HAL_UART_Receive_IT (up->huart, &up->buffer[9], 1);
    }

  if (!MyListIsEmpty (up->rx_list))
    {
      MyListItem_t *item = MyListPopFront (up->rx_list);

      for (size_t i = 0; i < up->callback_count; i++)
	{
	  if (memcmp (&item->data, &up->callbacks[i].cmd, 4) == 0)
	    {
	      up->callbacks[i].callback (up, (char*) &item->data);
	    }
	}

      vPortFree (item);
    }
}

void
UProto_SendData (UProto_t *up, char* data)
{
  MyListPushBack (up->tx_list, (uint8_t*)data);
  UProto_Transmit (up);
}

void
UProto_Receive_IT (UProto_t *up, UART_HandleTypeDef *huart)
{
  if (huart == up->huart
      && HAL_UART_GetState (huart) != HAL_UART_STATE_BUSY_RX)
    {
      memmove (&up->buffer, &up->buffer[1], 9);
      HAL_UART_Receive_IT (huart, &up->buffer[9], 1);

    }
}

void
UProto_Transmit_IT (UProto_t *up, UART_HandleTypeDef *huart)
{
  if (huart == up->huart)
    {
      UProto_Transmit (up);
    }
}

void
UProto_Transmit (UProto_t *up)
{
  if (HAL_UART_GetState (up->huart) != HAL_UART_STATE_BUSY_TX
      && !MyListIsEmpty (up->tx_list))
    {
      MyListItem_t *item = MyListPopFront (up->tx_list);

      up->tdata[0] = 0x01;
      up->tdata[1] = 0x02;
      memcpy (&up->tdata[2], &item->data, 5);
      up->tdata[7] = CountCRC8 (&up->tdata[2]);
      up->tdata[8] = 0x5A;

      vPortFree (item);
      HAL_UART_Transmit_DMA (up->huart, (uint8_t*)&up->tdata, 9);
    }
}

void
UProto_AddCallback (UProto_t *up, char* cmd, callback_ptr cb)
{
  UProtoCommand_t c = {0};
  memcpy(&c.cmd, cmd, strlen(cmd));
  c.callback = cb;

  up->callback_count ++;

  UProtoCommand_t *cmds = (UProtoCommand_t*) pvPortMalloc (
      sizeof(UProtoCommand_t) * up->callback_count);

  memcpy (cmds, up->callbacks, sizeof(UProtoCommand_t) * up->callback_count);
  cmds[up->callback_count - 1] = c;

  vPortFree (up->callbacks);
  up->callbacks = cmds;
}
