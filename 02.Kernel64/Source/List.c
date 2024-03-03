#include "List.h"

#include "Types.h"

inline void kInitializeList(List* list) {
  list->item_count = 0;
  list->head = nullptr;
  list->tail = nullptr;
}

inline size_t kGetListCount(const List* list) { return list->item_count; }

void kAddListToTail(List* list, void* item) {
  ListLink* now = (ListLink*)item;
  now->next = nullptr;

  if (!list->head) {
    list->head = item;
    list->tail = item;
    list->item_count = 1;
    return;
  }

  now = (ListLink*)list->tail;
  now->next = item;

  list->tail = item;
  ++list->item_count;
}

void kAddListToHeader(List* list, void* item) {
  ListLink* link = (ListLink*)item;
  link->next = list->head;

  if (!list->head) {
    list->head = item;
    list->tail = item;
    list->item_count = 1;
    return;
  }

  list->head = link;
  ++list->item_count;
}

void* kRemoveList(List* list, uint64 id) {
  ListLink* now;
  ListLink* prev;

  prev = list->head;
  for (now = prev; now != nullptr; now = now->next) {
    if (now->id == id) {
      if (now == list->head && now == list->tail) {
        list->head = nullptr;
        list->tail = nullptr;
      } else if (now == list->head) {
        list->head = now->next;
      } else if (now == list->tail) {
        list->tail = now->next;
      } else {
        prev->next = now->next;
      }

      --list->item_count;
      return now;
    }
    prev = now;
  }

  return nullptr;
}

inline void* kRemoveListFromHeader(List* list) {
  if (!list->item_count) {
    return nullptr;
  }
  ListLink* head = (ListLink*)list->head;
  return kRemoveList(list, head->id);
}

inline void* kRemoveListFromTail(List* list) {
  if (!list->item_count) {
    return nullptr;
  }
  ListLink* tail = (ListLink*)list->tail;
  return kRemoveList(list, tail->id);
}

void* kFindList(const List* list, uint64 id) {
  for (ListLink* now = (ListLink*)list->head; now != nullptr; now = now->next) {
    if (now->id == id) {
      return now;
    }
  }
  return nullptr;
}

inline void* kkGetHeaderFromList(const List* list) { return list->head; }

inline void* kGetTailFromList(const List* list) { return list->tail; }

inline void* kGetNextFromList(const List* list, void* current) {
  return ((ListLink*)current)->next;
}
