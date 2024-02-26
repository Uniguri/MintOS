#ifndef MINTOS_QUEUE_H_
#define MINTOS_QUEUE_H_

#include "Types.h"

#pragma pack(push, 1)
typedef struct kQueueMangerStruct {
  uint32 element_size;
  uint32 max_element_count;

  void* buffer;
  uint32 front_idx;
  uint32 back_idx;

  bool is_last_operation_push;
} Queue;
#pragma pack(pop)

// Initialize give queue.
// @param queue: pointer of queue to initialize.
// @param buffer: the buffer to store elements;
// sizeof(buffer)>=(max_element_count*element_size).
// @param max_element_count: the number of elements which queue can store.
// @param element_size: the size of each element.
void kInitializeQueue(Queue* queue, void* buffer, uint32 max_element_count,
                      uint32 element_size);
// @param queue: pointer of queue.
// @return the number of elements in queue.
uint32 kGetQueueSize(const Queue* queue);
// Check given queue is full.
// @param queue: pointer of queue.
// @return True if queue is full.
bool kIsQueueFull(const Queue* queue);
// Check given queue is empty.
// @param queue: pointer of queue.
// @return True if queue is empty.
bool kIsQueueEmpty(const Queue* queue);
// Push given element to queue.
// @param queue: pointer of queue.
// @param data: the pointer of element to push
// @return True if success.
bool kPushQueue(Queue* queue, const void* data);
// Get element from queue.
// @param queue: pointer of queue.
// @param data: the pointer of element to store
// @return True if success.
bool kGetFrontFromQueue(Queue* queue, void* data);
// Pop element from queue.
// @param queue: pointer of queue.
// @return True if success.
bool kPopQueue(Queue* queue);

#endif