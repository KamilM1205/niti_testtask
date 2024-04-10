#include <ibus.h>
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

IBus_t*
IBUS_Init (UART_HandleTypeDef *huart)
{
  IBus_t *ibus = (IBus_t*) pvPortMalloc (sizeof(IBus_t));
  memset (ibus, 0, sizeof(IBus_t));

  ibus->callbacks = pvPortMalloc (sizeof(IBusCommand_t));
  ibus->rx_list = MyListInit(IBUS_PACKAGE_LEN);
  ibus->huart = huart;

  return ibus;
}

uint8_t
isValidCRC (uint8_t *data)
{
  uint16_t crc = 0xFFFF;
  uint16_t big_endian_crc = data[31] << 8 | data[30];

  for (size_t i = 0; i < IBUS_PACKAGE_LEN-2; i++)
    {
      crc -= data[i];
    }

  return crc == big_endian_crc;
}

void
IBUS_FindSignature (IBus_t *up, uint8_t *buff)
{
  if (buff[0] == IBUS_PACKAGE_LEN && buff[1] == 0x40)
    {
      if (!isValidCRC(buff))
	{
	  return;
	}

      uint8_t data[IBUS_PACKAGE_LEN];
      memcpy (&data, buff, IBUS_PACKAGE_LEN);
      MyListPushBack (up->rx_list, (uint8_t*)&data);
      memset(&up->buffer, 0, IBUS_PACKAGE_LEN + 1);
    }
}

void
IBUS_Receive (IBus_t *up)
{
  IBUS_FindSignature (up, up->buffer);

  if (HAL_UART_GetState (up->huart) != HAL_UART_STATE_BUSY_RX)
    {
      HAL_UART_Receive_IT (up->huart, &up->buffer[IBUS_PACKAGE_LEN], 1);
    }

  if (!MyListIsEmpty (up->rx_list))
    {
      MyListItem_t *item = MyListPopFront (up->rx_list);

      uint8_t data[2];
      uint16_t num;

      for (size_t i = 0; i < up->callback_count; i++)
	{
	  memcpy(&data, &item->data[2 + 2 * up->callbacks[i].channel], 2);
	  num = data[1] << 8 | data[0];
	  up->callbacks[i].callback (up, num);
	}

      MyListDelete(item);
    }
}

void
IBUS_Receive_IT (IBus_t *up, UART_HandleTypeDef *huart)
{
  if (huart == up->huart
      && HAL_UART_GetState (huart) != HAL_UART_STATE_BUSY_RX)
    {
      memmove (&up->buffer, &up->buffer[1], IBUS_PACKAGE_LEN);
      HAL_UART_Receive_IT (huart, &up->buffer[IBUS_PACKAGE_LEN], 1);
    }
}

void
IBUS_AddCallback (IBus_t *up, IBUS_Channel channel, callback_ptr cb)
{
  IBusCommand_t c = {0};
  c.channel = channel;
  c.callback = cb;

  up->callback_count ++;

  IBusCommand_t *cmds = (IBusCommand_t*) pvPortMalloc (
      sizeof(IBusCommand_t) * up->callback_count);

  memcpy (cmds, up->callbacks, sizeof(IBusCommand_t) * up->callback_count);
  cmds[up->callback_count - 1] = c;

  vPortFree (up->callbacks);
  up->callbacks = cmds;
}
