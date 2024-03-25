/*
 * list.c
 *
 *  Created on: 17 мар. 2024 г.
 *      Author: Kamil
 */

#include "cmsis_os.h"
#include "ulist.h"
#include <string.h>

MyList_t*
MyListInit (void)
{
  MyList_t *list = (MyList_t*) pvPortMalloc (sizeof(MyList_t));
  memset (list, 0, sizeof(MyList_t));
  return list;
}

void
MyListConnect (MyListItem_t *a, MyListItem_t *b)
{
  if (a != NULL)
    {
      a->next = b;
    }

  if (b != NULL)
    {
      b->prev = a;
    }
}

void
MyListDisconnect (MyListItem_t *a, MyListItem_t *b)
{
  if (a != NULL)
    {
      a->next = NULL;
    }

  if (b != NULL)
    {
      b->prev = NULL;
    }
}

void
MyListInsert (MyList_t *list, uint8_t *data, size_t idx)
{
  MyListItem_t *item;
  MyListItem_t *new_item;

  item = list->first;

  if (idx >= list->size && list->size != 0)
    {
      return;
    }

  for (size_t i = 0; i < idx; i++)
    {
      item = item->next;
    }

  new_item = (MyListItem_t*) pvPortMalloc (sizeof(MyListItem_t));
  memcpy (&new_item->data, data, 9);
  new_item->next = NULL;
  new_item->prev = NULL;

  list->size++;

  if (list->first == NULL)
    {
      list->first = new_item;
      return;
    }

  MyListConnect (new_item, item->next);
  MyListConnect (item, new_item);
}

void
MyListPushBack (MyList_t *list, uint8_t *data)
{
  size_t idx = list->size;
  if (list->size != 0)
    {
      idx--;
    }

  MyListInsert (list, data, idx);
}

MyListItem_t*
MyListPopBack (MyList_t *list)
{
  MyListItem_t *item;

  if (list->size == 0)
    {
      return NULL;
    }

  item = MyListGet (list, list->size - 1);
  if (list->first == item)
    {
      list->first = NULL;
    }
  else
    {
      MyListDisconnect (item->prev, item);
    }
  list->size--;

  return item;
}

void
MyListPushFront (MyList_t *list, uint8_t *data)
{
  MyListInsert (list, data, 0);
}
MyListItem_t*
MyListPopFront (MyList_t *list)
{
  MyListItem_t *item;

  if (list->size == 0)
    {
      return NULL;
    }

  item = MyListGet (list, 0);
  MyListDisconnect (NULL, item);
  list->size--;

  if (list->size > 0)
    {
      list->first = item->next;
      MyListDisconnect (item, item->next);
    }
  else
    {
      list->first = NULL;
    }
  return item;
}

MyListItem_t*
MyListGet (MyList_t *list, size_t idx)
{
  MyListItem_t *item = list->first;

  if (idx >= list->size)
    {
      return NULL;
    }

  for (size_t i = 0; i < idx; i++)
    {
      item = item->next;
    }

  return item;
}

uint8_t
MyListIsEmpty (MyList_t *list)
{
  if (list->size > 0)
    {
      return 0;
    }

  return 1;
}

void
MyListDelete (MyList_t *list, size_t idx)
{
  MyListItem_t *item;

  if (idx >= list->size)
    {
      return;
    }

  item = list->first;

  MyListConnect (item->prev, item->next);

  list->size--;

  vPortFree (item);
}

void
MyListFree (MyList_t *list)
{
  MyListItem_t *item;
  MyListItem_t *next;

  item = list->first;

  do
    {
      next = item->next;
      vPortFree (item);
      item = next;
    }
  while (item != NULL);
}

