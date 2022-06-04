#include "../kernel/interrupt.h"
#include "../kernel/list.h"
#include "test.h"

IntrStatus intr_disable() { return INTR_OFF; }

IntrStatus intr_set_status(IntrStatus v) { return v; }

int main() {
  List list;

  list_init(&list);

  ASSERT(list_empty(&list));
  ASSERT(list_len(&list) == 0);

  ListElem *elemA = (ListElem *)malloc(sizeof(ListElem));
  ListElem *elemB = (ListElem *)malloc(sizeof(ListElem));
  ListElem *elemC = (ListElem *)malloc(sizeof(ListElem));
  ListElem *elemD = (ListElem *)malloc(sizeof(ListElem));
  ListElem *elemE = (ListElem *)malloc(sizeof(ListElem));

  list_append(&list, elemA);
  ASSERT(list_empty(&list) == false);
  ASSERT(list_len(&list) == 1);

  list_append(&list, elemB);
  ASSERT(list_empty(&list) == false);
  ASSERT(list_len(&list) == 2);

  list_push(&list, elemC);
  ASSERT(list_empty(&list) == false);
  ASSERT(list_len(&list) == 3);

  list_push(&list, elemD);
  ASSERT(list_empty(&list) == false);
  ASSERT(list_len(&list) == 4);

  ASSERT(list_find(&list, elemB));
  ASSERT(list_find(&list, elemE) == false);

  list_remove(elemB);
  ASSERT(list_find(&list, elemB) == false);

  list_remove(elemD);
  list_pop(&list);
  list_pop(&list);

  ASSERT(list_empty(&list));
  ASSERT(list_len(&list) == 0);
}