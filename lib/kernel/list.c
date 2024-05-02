#include "list.h"
#include "interrupt.h"

void list_init(List *list) {
  list->head.prev = NULL;
  list->head.next = &list->tail;
  list->tail.next = NULL;
  list->tail.prev = &list->head;
}

void list_insert_before(ListElem *before, ListElem *elem) {
  IntrStatus old =
      intr_disable(); // 变更链表结构操作不可被打断, 因此需要先关中断

  before->prev->next = elem;

  elem->prev = before->prev;
  elem->next = before;

  before->prev = elem;

  intr_set_status(old);
}

void list_push(List *plist, ListElem *elem) {
  list_insert_before(plist->head.next, elem);
}

void list_append(List *plist, ListElem *elem) {
  list_insert_before(&plist->tail, elem);
}

void list_remove(ListElem *elem) {
  IntrStatus old = intr_disable();

  elem->prev->next = elem->next;
  elem->next->prev = elem->prev;

  intr_set_status(old);
}

ListElem *list_pop(List *plist) {
  ListElem *elem = plist->head.next;
  list_remove(elem);
  return elem;
}

// 查找链表中的元素
bool list_find(List *plist, ListElem *obj) {
  for (ListElem *elem = plist->head.next; elem != &plist->tail;
       elem = elem->next) {
    if (elem == obj) {
      return true;
    }
  }
  return false;
}

ListElem *list_traversal(List *plist, list_func func, int arg) {
  if (list_empty(plist)) {
    return NULL;
  }

  ListElem *elem = plist->head.next;
  while (elem != &plist->tail) {
    if (func(elem, arg)) {
      return elem;
    }
    elem = elem->next;
  }

  return NULL;
}

uint32_t list_len(List *plist) {
  int length = 0;
  for (ListElem *elem = plist->head.next; elem != &plist->tail;
       elem = elem->next) {
    length++;
  }
  return length;
}

bool list_empty(List *plist) { return plist->head.next == &plist->tail; }
