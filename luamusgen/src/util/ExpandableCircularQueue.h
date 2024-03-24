//
// Created by rutio on 15.11.19.
//

#ifndef LUAMUSGEN_CIRCULARQUEUE_H
#define LUAMUSGEN_CIRCULARQUEUE_H

#include <algorithm>

namespace util {

  template<class T>
  class ExpandableCircularQueue {
  public:
    explicit ExpandableCircularQueue(size_t initial_capacity = 64) {
      if (initial_capacity == 0) {
        array = nullptr;
        array_length = 0;
        length = 0;
        head = 0;
        tail = 0;
      } else {
        array = new T[initial_capacity];
        array_length = initial_capacity;
        length = 0;
        head = 0;
        tail = initial_capacity - 1;
      }
    }

    ExpandableCircularQueue(const ExpandableCircularQueue& other) {
      copy_from(other);
    }

    ExpandableCircularQueue& operator=(const ExpandableCircularQueue& other) {
      delete[] array;
      copy_from(other);
      return *this;
    }

    ExpandableCircularQueue(ExpandableCircularQueue&& other) noexcept {
      array = other.array;
      array_length = other.array_length;
      length = other.length;
      head = other.head;
      tail = other.tail;
      other.array = nullptr;
      other.array_length = 0;
      other.length = 0;
      other.head = 0;
      other.tail = 0;
    }

    ExpandableCircularQueue& operator=(ExpandableCircularQueue&& other) noexcept {
      delete[] array;
      array = other.array;
      array_length = other.array_length;
      length = other.length;
      head = other.head;
      tail = other.tail;
      other.array = nullptr;
      other.array_length = 0;
      other.length = 0;
      other.head = 0;
      other.tail = 0;
      return *this;
    }

    inline size_t capacity() { return array_length; }

    inline size_t size() { return length; }

    inline bool empty() { return length == 0; }

    inline T& front() { return array[head]; }

    inline T& back() { return array[tail]; }

    inline void pop_front() {
      length--;
      head++;
      if (head >= array_length) {
        head = 0;
      }
    }

    inline void pop_back() {
      length--;
      if (tail == 0) {
        tail = array_length - 1;
      } else {
        tail--;
      }
    }

    inline void push_front(const T& v) {
      length++;
      if (length > array_length) {
        reserve(std::max(array_length * 2, (size_t) 1));
      }
      if (head == 0) {
        head = array_length - 1;
      } else {
        head--;
      }
      array[head] = v;
    }

    inline void push_front(T&& v) {
      length++;
      if (length > array_length) {
        reserve(std::max(array_length * 2, (size_t) 1));
      }
      if (head == 0) {
        head = array_length - 1;
      } else {
        head--;
      }
      array[head] = std::move(v);
    }

    inline void push_back(const T& v) {
      length++;
      if (length > array_length) {
        reserve(std::max(array_length * 2, (size_t) 1));
      }
      tail++;
      if (tail >= array_length) {
        tail = 0;
      }
      array[tail] = v;
    }

    inline void push_back(T&& v) {
      length++;
      if (length > array_length) {
        reserve(std::max(array_length * 2, (size_t) 1));
      }
      tail++;
      if (tail >= array_length) {
        tail = 0;
      }
      array[tail] = std::move(v);
    }

    inline void reserve(size_t new_capacity) {
      if (array == nullptr) {
        array = new T[new_capacity];
        array_length = new_capacity;
        length = 0;
        head = 0;
        tail = new_capacity - 1;
        return;
      }

      T* new_array = new T[new_capacity];
      T* new_tail = new_array;
      if (head <= tail) {
        for (T* p = array + head, * e = array + tail; p <= e; ++p, ++new_tail) {
          *new_tail = std::move(*p);
        }
      } else if (length > 0) {
        for (T* p = array + head, * e = array + array_length; p < e; ++p, ++new_tail) {
          *new_tail = std::move(*p);
        }
        for (T* p = array, * e = array + tail; p <= e; ++p, ++new_tail) {
          *new_tail = std::move(*p);
        }
      }
      array = new_array;
      array_length = new_capacity;
      head = 0;
      if (new_tail == new_array) {
        tail = array_length - 1;
      } else {
        tail = new_tail - array - 1;
      }
    }

  private:
    inline void copy_from(const ExpandableCircularQueue<T>& other) {
      if (other.array == nullptr) {
        array = nullptr;
        array_length = 0;
        length = 0;
        head = 0;
        tail = 0;
        return;
      }

      array = new T[other.array_length];
      array_length = other.array_length;
      length = other.length;
      T* new_tail = array;
      if (other.head <= other.tail) {
        for (T* p = other.array + other.head, *e = other.array + other.tail; p <= e; ++p, ++new_tail) {
          *new_tail = std::move(*p);
        }
      } else if (length > 0) {
        for (T* p = other.array + other.head, *e = other.array + other.array_length; p < e; ++p, ++new_tail) {
          *new_tail = std::move(*p);
        }
        for (T* p = other.array, *e = other.array + other.tail; p <= e; ++p, ++new_tail) {
          *new_tail = std::move(*p);
        }
      }
      head = 0;
      if (new_tail == array) {
        tail = array_length - 1;
      } else {
        tail = new_tail - array - 1;
      }
    }

    T* array;
    size_t array_length;
    size_t length;
    size_t head;
    size_t tail;
  };

}


#endif //LUAMUSGEN_CIRCULARQUEUE_H
