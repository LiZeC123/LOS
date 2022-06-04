#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct _list_elem {
  struct _list_elem *prev;
  struct _list_elem *next;
};

typedef struct _list_elem ListElem;

struct _list {
  ListElem head;
  ListElem tail;
};

typedef struct _list List;

typedef bool (*list_func)(ListElem *elem, int arg);

void list_init(List *list);

void list_insert_before(ListElem *before, ListElem *elem);

// 在链表的头部插入一个元素
void list_push(List *plist, ListElem *elem);

// 在链表的尾部插入一个元素
void list_append(List *plist, ListElem *elem);

void list_remove(ListElem *elem);

// 弹出链表头部的一个元素
ListElem *list_pop(List *plist);

bool list_empty(List *list);

uint32_t list_len(List *list);

// 返回满足条件的一个元素
ListElem *list_traversal(List *plist, list_func func, int arg);

bool list_find(List *plist, ListElem *obj);
