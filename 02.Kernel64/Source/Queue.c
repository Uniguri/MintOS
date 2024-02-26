#include "Queue.h"

#include "Memory.h"
#include "Types.h"

inline void kInitializeQueue(Queue* queue, void* buffer,
                             uint32 max_element_count, uint32 element_size) {
  queue->buffer = buffer;
  queue->max_element_count = max_element_count;
  queue->element_size = element_size;

  queue->front_idx = 0;
  queue->back_idx = 0;
  queue->is_last_operation_push = false;
}

inline uint32 kGetQueueSize(const Queue* queue) {
  if (kIsQueueEmpty(queue)) {
    return 0;
  }

  uint32 maybe_size = queue->back_idx - queue->front_idx;
  return maybe_size > 0 ? maybe_size : queue->max_element_count + maybe_size;
}

inline bool kIsQueueFull(const Queue* queue) {
  return queue->front_idx == queue->back_idx && queue->is_last_operation_push;
}

inline bool kIsQueueEmpty(const Queue* queue) {
  return queue->front_idx == queue->back_idx && !queue->is_last_operation_push;
}

bool kPushQueue(Queue* queue, const void* data) {
  if (kIsQueueFull(queue)) {
    return false;
  }

  char* buffer = (char*)queue->buffer;
  const uint32 target_offset = queue->element_size * queue->back_idx;
  memcpy(buffer + target_offset, data, queue->element_size);

  if (++queue->back_idx >= queue->max_element_count) {
    queue->back_idx %= queue->max_element_count;
  }
  queue->is_last_operation_push = true;
  return true;
}

inline bool kGetFrontFromQueue(Queue* queue, void* data) {
  if (kIsQueueEmpty(queue)) {
    return false;
  }

  const char* buffer = (char*)queue->buffer;
  const uint32 target_offset = queue->front_idx * queue->element_size;
  memcpy(data, buffer + target_offset, queue->element_size);

  return true;
}

inline bool kPopQueue(Queue* queue) {
  if (kIsQueueEmpty(queue)) {
    return false;
  }

  if (++queue->front_idx >= queue->max_element_count) {
    queue->front_idx %= queue->max_element_count;
  }
  queue->is_last_operation_push = false;
  return true;
}