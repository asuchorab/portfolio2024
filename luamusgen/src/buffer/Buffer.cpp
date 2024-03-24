//
// Created by rutio on 24.02.19.
//

#include <util/Logger.h>
#include "Buffer.h"

Buffer::Buffer(double sample_rate): sample_rate(sample_rate) {
}

void Buffer::requestSpace(int64_t start, int64_t length) {
  if (length < 0) {
    return;
  }
  int64_t end = start + length;
  if (offset == std::numeric_limits<int64_t>::max()) {
    offset = start;
    end_offset = end;
  } else {
    offset = std::min(offset, start);
    end_offset = std::max(end_offset, end);
  }
}

bool Buffer::allocate() {
  if (!data.empty()) {
    logErrorC("attempted reallocation");
    return false;
  }
  if (end_offset <= offset) {
    logWarningC("end_offset <= offset");
  } else {
    try {
      data.resize(static_cast<size_t>(end_offset - offset));
    } catch (const std::bad_alloc&) {
      logErrorC("bad allocation caught")
      return false;
    }
  }
  return true;
}

bool Buffer::deallocate() {
  std::lock_guard<std::mutex> lock(transform_register_mutex);
  if (!transform_register.empty()) {
    logErrorC("Tried to deallocate when buffer was being used")
    return false;
  }
  std::vector<double>().swap(data);
  return true;
}

bool Buffer::isReady() {
  return !data.empty();
}

const Buffer::TransformData* Buffer::registerTransform(int64_t start, int64_t length) {
  std::lock_guard<std::mutex> lock(transform_register_mutex);
  if (data.empty()) {
    if (!allocate()) {
      return nullptr;
    }
  }
  int64_t end = start + length;
  for (auto &t : transform_register) {
    if (t->start < end && t->end > start) {
      return nullptr;
    }
  }
  if (start < offset && end > end_offset) {
    throw std::runtime_error("buffer transform out of bounds request");
  }

  std::unique_ptr<TransformData> result(new TransformData());
  result->buffer = this;
  result->start = start;
  result->end = end;
  result->length = length;
  result->data_pointer = &data[start - offset];

  const TransformData* transformDataPointer = result.get();
  transform_register.emplace_front(std::move(result));

  return transformDataPointer;
}

void Buffer::releaseTransform(const Buffer::TransformData* transform) {
  std::lock_guard<std::mutex> lock(transform_register_mutex);
  auto remove_it = transform_register.end();
  for (auto it = transform_register.begin(); it != transform_register.end(); ++it) {
    if (transform == it->get()) {
      remove_it = it;
      break;
    }
  }
  if (remove_it != transform_register.end()) {
    transform_register.erase(remove_it);
  } else {
    logWarningC("reuqested deletion of a nonexistent transform");
  }
}

void Buffer::TransformData::release() const {
  buffer->releaseTransform(this);
}
