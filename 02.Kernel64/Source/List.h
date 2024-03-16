#ifndef MINTOS_LIST_H_
#define MINTOS_LIST_H_

#include "Types.h"

#pragma pack(push, 1)
typedef struct kListLinkStruct {
  void* next;
  uint64 id;
} ListLink;

typedef struct kListManagerStruct {
  size_t item_count;
  void* head;
  void* tail;
} List;
#pragma pack(pop)

void kInitializeList(List* list);
size_t kGetListCount(const List* list);
void kAddListToTail(List* list, void* item);
void kAddListToHead(List* list, void* item);
// Remove element with id from list and return it.
// @param list: pointer to list.
// @id: id to remove element.
// @return valid pointer if success to remove. If fail, return nullptr.
void* kRemoveList(List* list, uint64 id);
// Remove head element from list and return it.
// @param list: pointer to list.
// @return valid pointer if success to remove. If fail, return nullptr.
void* kRemoveListFromHead(List* list);
// Remove tail element from list and return it.
// @param list: pointer to list.
// @return valid pointer if success to remove. If fail, return nullptr.
void* kRemoveListFromTail(List* list);
void* kFindList(const List* list, uint64 id);
void* kkGetHeaderFromList(const List* list);
void* kGetTailFromList(const List* list);
void* kGetNextFromList(const List* list, void* current);

#endif