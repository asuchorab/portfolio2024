//
// Created by rutio on 24.02.19.
//

#ifndef LUAMUSGEN_BUFFER_H
#define LUAMUSGEN_BUFFER_H

#include <memory>
#include <mutex>
#include <vector>
#include <list>

/**
 * Represents an audio buffer. It can be grown at node creating stage.
 * It also makes sure all transforms running at execution in parallel
 * don't collide.
 */
class Buffer {
public:

  class TransformData {
  public:
    void release() const;
    double* data_pointer;
    int64_t length;
  private:
    Buffer* buffer;
    int64_t start;
    int64_t end;
    friend class Buffer;
  };

  explicit Buffer(double sample_rate);

  void requestSpace(int64_t start, int64_t len);
  bool deallocate();
  bool isReady();
  const TransformData* registerTransform(int64_t start, int64_t length);
  void releaseTransform(const TransformData* transform);
  inline double getSampleRate() { return sample_rate; }
  inline int64_t getSize() { return data.size(); }
  inline double& operator[](size_t i) { return data[i]; }

private:
  bool allocate();
  std::vector<double> data;
  std::list<std::unique_ptr<Buffer::TransformData>> transform_register;
  std::mutex transform_register_mutex;
  int64_t offset = std::numeric_limits<int64_t>::max();
  int64_t end_offset = 0;
  double sample_rate;
};


#endif //LUAMUSGEN_BUFFER_H
