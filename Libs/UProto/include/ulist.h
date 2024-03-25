/*
 * list.h
 *
 *  Created on: 17 мар. 2024 г.
 *      Author: Kamil
 */

#ifndef _ULIST_H_
#define _ULIST_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct MyListItem
{
  uint8_t data[5];
  struct MyListItem *next;
  struct MyListItem *prev;
} MyListItem_t;

typedef struct MyList
{
  MyListItem_t *first;
  size_t size;
} MyList_t;

MyList_t*
MyListInit (void);
void
MyListConnect (MyListItem_t *a, MyListItem_t *b);
void
MyListDisconnect (MyListItem_t *a, MyListItem_t *b);
void
MyListInsert (MyList_t *list, uint8_t *data, size_t idx);
void
MyListPushBack (MyList_t *list, uint8_t *data);
MyListItem_t*
MyListPopBack (MyList_t *list);
void
MyListPushFront (MyList_t *list, uint8_t *data);
MyListItem_t*
MyListPopFront (MyList_t *list);
MyListItem_t*
MyListGet (MyList_t *list, size_t idx);
uint8_t
MyListIsEmpty (MyList_t *list);
void
MyListDelete (MyList_t *list, size_t idx);
void
MyListFree (MyList_t *list);

#endif /* _ULIST_H_ */
