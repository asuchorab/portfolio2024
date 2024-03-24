#ifndef LIBNEBULABROTGEN_H
#define LIBNEBULABROTGEN_H

#include "stb_image_write.h"
#include <memory>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <mutex>
#include <memory>
#include <complex>
#include <algorithm>
#include "buddhabrotrenderer.hpp"

namespace nebulabrotgen {

/**
 *
 * Holds data of one channel in the rendering, as matrix of counters
 * @tparam counter_t type for integer counter, for longer renders 16bit may not be enough
 */
template<class counter_t = uint16_t>
class NebulabrotChannelBuffer {
public:
  NebulabrotChannelBuffer(size_t width, size_t height)
      : completed_iterations(0), data(width * height),
        max_value(0), mergeMutex(new std::mutex()) {}

  NebulabrotChannelBuffer(const NebulabrotChannelBuffer& other) {
    completed_iterations = other.completed_iterations;
    data = other.data;
    max_value = other.max_value;
    mergeMutex = std::make_unique<std::mutex>();
  }

  NebulabrotChannelBuffer& operator=(const NebulabrotChannelBuffer& other) {
    completed_iterations = other.completed_iterations;
    data = other.data;
    max_value = other.max_value;
    mergeMutex = std::make_unique<std::mutex>();
    return *this;
  }

  NebulabrotChannelBuffer(NebulabrotChannelBuffer&& other) noexcept {
    completed_iterations = other.completed_iterations;
    data = std::move(other.data);
    max_value = other.max_value;
    mergeMutex = std::make_unique<std::mutex>();
  }

  NebulabrotChannelBuffer& operator=(NebulabrotChannelBuffer&& other) noexcept {
    completed_iterations = other.completed_iterations;
    data = std::move(other.data);
    max_value = other.max_value;
    mergeMutex = std::make_unique<std::mutex>();
    return *this;
  }

  void clear() {
    std::fill(data.begin(), data.end(), 0);
  }

  counter_t* getData() {
    return data.data();
  }

  counter_t getMaxValue() const {
    std::lock_guard<std::mutex> lock(*mergeMutex);
    return max_value;
  }

  bool mergeWith(const NebulabrotChannelBuffer& other) {
    std::lock_guard<std::mutex> lock(*mergeMutex);
    size_t mem_size = data.size();
    if (mem_size != other.data.size()) {
      return false;
    }
    for (size_t i = 0; i < mem_size; ++i) {
      data[i] += other.data[i];
    }
    completed_iterations += other.completed_iterations;
    return true;
  }

  bool toStream(std::ostream& os) {
    os.write((char*) &completed_iterations, sizeof(size_t));
    os.write((char*) &max_value, sizeof(size_t));
    os.write((char*) data.data(), data.size() * sizeof(uint32_t));
    return os.good();
  }

  bool fromStream(std::istream& is) {
    is.read((char*) &completed_iterations, sizeof(size_t));
    is.read((char*) &max_value, sizeof(size_t));
    is.read((char*) data.data(), data.size() * sizeof(uint32_t));
    return is.good();
  }

  void updateMaxValue() {
    std::lock_guard<std::mutex> lock(*mergeMutex);
    max_value = 0;
    size_t mem_size = data.size();
    for (size_t i = 0; i < mem_size; ++i) {
      if (data[i] > max_value) {
        max_value = data[i];
      }
    }
  }

  size_t getCompletedIterations() {
    return completed_iterations;
  }

  void addCompletedIterations(size_t amount) {
    completed_iterations += amount;
  }

private:
  std::vector<counter_t> data;
  std::unique_ptr<std::mutex> mergeMutex;
  size_t completed_iterations;
  counter_t max_value;
};

/**
 * Holds multiple channel buffers together
 * @tparam counter_t type for integer counter, for longer renders 16bit may not be enough
 */
template<class counter_t = uint16_t>
class NebulabrotChannelCollection {
public:
  NebulabrotChannelCollection(size_t width, size_t height)
      : width(width), height(height) {}

  bool loadFile(const std::string& filename) {
    auto fs = std::fstream(filename, std::ios::in | std::ios::binary);
    if (!fs.is_open()) {
      std::cout << "Unable to open raw results file: " << filename << "\n";
      return false;
    }
    size_t read_width;
    size_t read_height;
    fs.read((char*) &read_width, sizeof(read_width));
    fs.read((char*) &read_height, sizeof(read_height));
    if (!fs.good()) {
      std::cout << "Error while reading raw results file: " << filename << "\n";
      fs.close();
      return false;
    }
    if (width != read_width || height != read_height) {
      std::cout << "Error while loading: " << filename << ", resolution mismatch\n";
      fs.close();
    }
    std::string channels_info;
    while (true) {
      NebulabrotChannelBuffer buf(width, height);
      size_t read_name_length = 0;
      std::string name;

      fs.read((char*) &read_name_length, sizeof(read_name_length));
      if (read_name_length >= 1024) {
        std::cout << "Warning: channel name is " << read_name_length << " bytes long\n";
      }
      name.resize(read_name_length);
      fs.read(&name[0], (long long) read_name_length);

      if (!fs.good()) {
        if (fs.eof()) {
          break;
        } else {
          std::cout << "Error while loading: " << filename << "\n";
          fs.close();
          return false;
        }
      }
      if (!buf.fromStream(fs)) {
        if (fs.eof()) {
          std::cout << "Error while loading " << name << " from " << filename << ", EoF reached\n";
        } else {
          std::cout << "Error while loading " << name << " from " << filename << "\n";
        }
      }
      if (!channels_info.empty()) {
        channels_info += ", ";
      }
      auto it = channels.find(name);
      if (it == channels.end()) {
        channels.emplace(name, std::move(buf));
        channels_info += name;
      } else {
        if (!it->second.mergeWith(buf)) {
          std::cout << "Error while loading and merging " << name << " from " << filename
                    << ": this should never happen\n";
        }
        it->second.updateMaxValue();
        channels_info += name + "(merged)";
      }
    }
    fs.close();
    std::cout << "Loaded raw results file: " << filename << ", channels: " << channels_info << "\n";
    return true;
  }

  bool saveFile(const std::string& filename) {
    auto fs = std::fstream(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!fs.is_open()) {
      std::cout << "Unable to create raw results file: " << filename << "\n";
      return false;
    }
    fs.write((char*) &width, sizeof(width));
    fs.write((char*) &height, sizeof(height));
    std::string channels_info;
    for (auto& p : channels) {
      size_t string_len = p.first.size();
      fs.write((char*) &string_len, sizeof(string_len));
      fs.write(&p.first[0], string_len);
      if (!p.second.toStream(fs)) {
        std::cout << "Error while saving raw results file: " << filename << "\n";
        fs.close();
        return false;
      }
      if (!channels_info.empty()) {
        channels_info += ", ";
      }
      channels_info += p.first;
    }
    fs.close();
    std::cout << "Saved raw results file: " << filename << ", channels: " << channels_info << "\n";
    return true;
  }

  void merge(const NebulabrotChannelCollection& other) {
    std::string channels_info;
    for (auto& p : other.channels) {
      if (!channels_info.empty()) {
        channels_info += ", ";
      }
      auto it = channels.find(p.first);
      if (it == channels.end()) {
        channels.emplace(p.first, p.second);
        channels_info += p.first;
      } else {
        it->second.mergeWith(p.second);
        it->second.updateMaxValue();
        channels_info += p.first + "(merged)";
      }
    }
    std::cout << "Merged channel collection: " + channels_info + "\n";
  }

  std::map<std::string, NebulabrotChannelBuffer<counter_t>> channels;

  inline size_t getWidth() const { return width; }

  inline size_t getHeight() const { return height; }

private:
  size_t width;
  size_t height;
};

/**
 * Structure containing function with its approximate relative cost,
 * generally 1 is good but for complicated functions it's a way to influence
 * multithreading work division
 * @tparam real_t type representing real numbers
 */
template<class real_t = float>
struct InnerFunctionData {
  typedef void(* InnerFunc)(std::complex<real_t>&, std::complex<real_t>);

  explicit InnerFunctionData(InnerFunc ptr, real_t cost = 1.0)
      : ptr(ptr), cost(cost) {}

  InnerFunc ptr;
  real_t cost;
};

/**
 * Structure describing computation layer
 * @tparam real_t type representing real numbers
 */
template<class real_t = float>
struct NebulabrotIterationData {
  /**
   * Creates computation layer description
   * @param inner_iterations amount of iterations of the function in each pass, analogous to mandelbrot iterations
   * @param renderer_iterations amount of
   * @param func function to use for rendering
   */
  NebulabrotIterationData(size_t inner_iterations, size_t renderer_iterations,
                          const InnerFunctionData<real_t>& func)
      : inner_iterations(inner_iterations),
        renderer_iterations(renderer_iterations), func(func) {}

  real_t getCost() const {
    return func.cost * renderer_iterations *
           (inner_iterations + (real_t) 128.0 * std::pow((real_t) 2.0, inner_iterations / (real_t) 1024.0));
  }

  size_t inner_iterations;
  size_t renderer_iterations;
  InnerFunctionData<real_t> func;
};

/**
 * Structure corresponding to a rendering layer with its configuration
 * and buffers
 * @tparam real_t type representing real numbers
 * @tparam counter_t type for integer counter, for longer renders 16bit may not be enough
 */
template<class real_t = float, class counter_t = uint16_t>
struct NebulabrotRenderChannel {
  NebulabrotRenderChannel(const NebulabrotIterationData<real_t>& data,
                          std::string name)
      : name(std::move(name)), data(data) {
    cost = data.getCost();
  }

  inline bool operator<(const NebulabrotRenderChannel& other) const {
    return cost < other.cost;
  }

  real_t cost;
  std::string name;
  NebulabrotIterationData<real_t> data;
  NebulabrotChannelBuffer<counter_t>* buf;
  size_t unfinished_jobs = 0;
  size_t threads_on_channel = 0;
  std::vector<size_t> iteration_jobs;
};

/**
 * Structure corresponding to a piece of work for a thread
 * @tparam real_t type representing real numbers
 * @tparam counter_t type for integer counter, for longer renders 16bit may not be enough
 */
template<class real_t = float, class counter_t = uint16_t>
struct IterJobData {
  IterJobData()
      : iter_data(0, 0, InnerFunctionData<real_t>(nullptr, 0)) {}

  NebulabrotIterationData<real_t> iter_data;
  NebulabrotChannelBuffer<counter_t>* buf = nullptr;
  size_t num_channel = 0;
};

/**
 * Main renderer class, after adding channels call execute
 * @tparam real_t type representing real numbers
 * @tparam counter_t type for integer counter, for longer renders 16bit may not be enough
 */
template<class real_t = float, class counter_t = uint16_t>
class NebulabrotRenderingManager {
public:
  NebulabrotRenderingManager(real_t xmid, real_t ymid, real_t factor,
                             real_t random_radius, real_t norm_limit,
                             size_t width, size_t height, size_t num_threads)
      : xmid(xmid), ymid(ymid), factor(factor),
        random_radius(random_radius), norm_limit(norm_limit),
        width(width), height(height), num_threads(num_threads) {}

  /**
   * Adds a rendering channel/layer
   * @param name unique name, later used in image saving
   * @param iteration_data configuration for the renderer
   */
  void add(const std::string& name,
           const NebulabrotIterationData<real_t>& iteration_data) {
    auto insert_it = channels.begin();
    for (auto it = insert_it; it != channels.end(); ++it) {
      int comp_result = name.compare(it->name);
      if (comp_result == 0) {
        throw std::invalid_argument(
            "Error while adding iteration channel to rendering manager: name conflict (" + name + ")");
      } else if (comp_result < 0) {
        insert_it = it + 1;
      }
    }
    channels.emplace(insert_it, iteration_data, name);
  }

  /**
   * Does the nebulabrot rendering in multiple threads, returns synchronously
   * @return channel (buffer) collection that can be later used to save an image
   */
  NebulabrotChannelCollection<counter_t> execute() {
    std::lock_guard<std::mutex> lock(execute_mutex);
    NebulabrotChannelCollection result(width, height);
    if (channels.empty()) {
      return result;
    }
    render_start = std::chrono::high_resolution_clock::now();
    std::string starting_message = "Computing fractal (";
    std::sort(channels.begin(), channels.end());
    double total_cost = 0.0;
    for (auto& ch : channels) {
      total_cost += ch.cost;
    }
    size_t approx_num_jobs = num_threads * 3 + static_cast<size_t>(std::log2(total_cost));
    jobs_total = 0;
    jobs_finished = 0;
    last_notification_elapsed = 0;
    for (auto& ch : channels) {
      if (ch.data.inner_iterations < 2) {
        std::cout << "Channel " + ch.name + " has less than 2 inner iterations, the rendering would never end\n";
        continue;
      }
      auto it = result.channels.emplace_hint(result.channels.end(), ch.name, NebulabrotChannelBuffer(width, height));
      ch.buf = &it->second;
      size_t ch_jobs = std::max((size_t) 1, (size_t) (ch.cost / total_cost * approx_num_jobs));
      size_t iterations_per_job_base = ch.data.renderer_iterations / ch_jobs;
      size_t iterations_per_job_rem = ch.data.renderer_iterations % ch_jobs;
      ch.iteration_jobs.resize(ch_jobs);
      ch.unfinished_jobs = ch_jobs;
      ch.threads_on_channel = 0;
      for (size_t i = 0; i < ch_jobs; ++i) {
        if (i < iterations_per_job_rem) {
          ch.iteration_jobs[i] = iterations_per_job_base + 1;
        } else {
          ch.iteration_jobs[i] = iterations_per_job_base;
        }
      }
      if (jobs_total > 0) {
        starting_message += ", ";
      }
      starting_message += ch.name;
      jobs_total += ch_jobs;
    }
    starting_message += ")\n";
    if (jobs_total == 0) {
      std::cout << "No channels to render\n";
      return result;
    } else {
      std::cout << starting_message;
    }
    std::vector<std::thread> threads;
    size_t temp_channel_num = channels.size() - 1;
    {
      std::lock_guard<std::mutex> lock1(leave_mutex);
      for (size_t i = 0; i < num_threads; ++i) {
        channels[temp_channel_num].threads_on_channel++;
        threads.emplace_back(&NebulabrotRenderingManager::threadFunction, this, temp_channel_num);
        if (temp_channel_num == 0) {
          temp_channel_num = channels.size() - 1;
        } else {
          temp_channel_num--;
        }
      }
    }
    for (size_t i = 0; i < num_threads; ++i) {
      threads[i].join();
    }
    double time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - render_start).count();
    std::cout << "Computing ended in " << time << std::endl;
    return result;
  }

  const static size_t NO_CHANNEL = (size_t) -1;

private:
  void threadFunction(size_t start_channel) {
    std::unique_ptr<BuddhabrotRenderer<real_t, counter_t>> renderer;
    size_t previous_channel = NO_CHANNEL;
    NebulabrotChannelBuffer buf(width, height);
    while (true) {
      IterJobData job = getAJob(start_channel);
      if (job.iter_data.renderer_iterations == 0) {
        if (previous_channel < channels.size()) {
          leaveChannel(previous_channel, NO_CHANNEL, buf);
        }
        return;
      }
      start_channel = job.num_channel;
      if (previous_channel != start_channel) {
        renderer.reset(new BuddhabrotRenderer<real_t, counter_t>
                           (width, height, job.iter_data.inner_iterations, 1, job.iter_data.func.ptr, random_radius,
                            norm_limit));
        renderer->setArea(xmid, ymid, factor);
        try {
          renderer->prepareInitialPoints();
        } catch (const std::runtime_error& e) {
          std::cout << std::string(e.what()) + "\n";
          return;
        }
        if (previous_channel != NO_CHANNEL) {
          leaveChannel(previous_channel, start_channel, buf);
          buf.clear();
        }
      }
      renderer->outputPointValues(buf.getData(), job.iter_data.renderer_iterations);
      buf.addCompletedIterations((counter_t) job.iter_data.renderer_iterations);
      previous_channel = start_channel;
      notifyJobCompletion(start_channel);
    }
  }

  IterJobData<real_t, counter_t> getAJob(size_t preferred_channel) {
    std::lock_guard<std::mutex> lock(job_getter_mutex);
    IterJobData result;
    bool found = false;
    size_t channels_size = channels.size();
    for (size_t i = 0; i < channels_size; ++i) {
      size_t vec_size = channels[preferred_channel].iteration_jobs.size();
      if (vec_size > 0) {
        result.iter_data.renderer_iterations =
            channels[preferred_channel].iteration_jobs[vec_size - 1];
        channels[preferred_channel].iteration_jobs.pop_back();
        found = true;
        break;
      }
      if (preferred_channel > 0) {
        preferred_channel--;
      } else {
        preferred_channel = channels_size - 1;
      }
    }
    if (!found) {
      return result;
    }
    result.iter_data.inner_iterations =
        channels[preferred_channel].data.inner_iterations;
    result.iter_data.func = channels[preferred_channel].data.func;
    result.num_channel = preferred_channel;
    result.buf = channels[preferred_channel].buf;
    return result;
  }

  void notifyJobCompletion(size_t channel_id) {
    std::lock_guard<std::mutex> lock(notify_mutex);
    channels[channel_id].unfinished_jobs--;
    jobs_finished++;
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - render_start).count();
    int elapsed_int = static_cast<int>(elapsed);
    if (elapsed_int != last_notification_elapsed) {
      last_notification_elapsed = elapsed_int;
      if (jobs_finished < jobs_total) {
        double approx_job_proggress = jobs_finished + std::min(num_threads, jobs_total - jobs_finished) / 3.0;
        double estimated = elapsed * (jobs_total - approx_job_proggress) / approx_job_proggress;
        std::cout << "(" << jobs_finished << "/" << jobs_total << ") Elapsed time: " << elapsed
                  << ", estimated remaining time: " << estimated << "\n";
      }
    }
  }

  void leaveChannel(size_t previous_channel, size_t new_channel,
                    const NebulabrotChannelBuffer<counter_t>& buf) {
    channels[previous_channel].buf->mergeWith(buf);
    std::lock_guard<std::mutex> lock(leave_mutex);
    if (new_channel != NO_CHANNEL) {
      channels[new_channel].threads_on_channel++;
    }
    if (previous_channel != NO_CHANNEL) {
      channels[previous_channel].threads_on_channel--;
      if (channels[previous_channel].threads_on_channel == 0 && channels[previous_channel].unfinished_jobs == 0) {
        channels[previous_channel].buf->updateMaxValue();
      }
    }
  }

  std::vector<NebulabrotRenderChannel<real_t, counter_t>> channels;
  std::mutex execute_mutex;
  std::mutex job_getter_mutex;
  std::mutex notify_mutex;
  std::mutex leave_mutex;
  std::chrono::time_point<std::chrono::high_resolution_clock> render_start;
  int last_notification_elapsed = 0;
  size_t jobs_total = 0;
  size_t jobs_finished = 0;
  real_t xmid;
  real_t ymid;
  real_t factor;
  real_t random_radius;
  real_t norm_limit;
  size_t width;
  size_t height;
  size_t num_threads;
};

/**
 * Representation of RGBA image
 */
class ImageColorBuffer {
public:
  ImageColorBuffer(size_t width, size_t height);

  inline uint32_t* getData() { return data.data(); }

  bool saveFile(const std::string& filename);

private:
  size_t width;
  size_t height;
  std::vector<uint32_t> data;
};

enum ImageMode {
  PIXEL_FUNC = 0, IMAGE_FUNC = 1
};

/**
 * Represents the way to convert frequency matrix of the fractal into image
 * @tparam real_t type representing real numbers
 * @tparam counter_t type for integer counter, for longer renders 16bit may not be enough
 */
template<class real_t = float, class counter_t = uint16_t>
struct ImageFunctionData {
  /**
   * return: RGBA value
   * arg1: array of values corresponding to channels (val/max)
   */
  typedef uint32_t (* ImagePixelFunc)(real_t*);

  /**
   * arg1: amount of pixels
   * arg2: array of pointers to iterations results for each channel
   * arg3: array of maximum values for each channel
   * arg4: result pointer to RGBA
   */
  typedef void (* WholeImageFunc)(size_t, counter_t**, counter_t*, uint32_t*);

  union ImageFunc {
    ImagePixelFunc pixel;
    WholeImageFunc whole;
  };

  /**
   * Defines a function that works per pixel
   * @param ptr function pointer
   * @param channel_names which channels out of a channel collection to use
   * @param desired_max value of matrix for each channel that corresponds to
   * maximum value on image, the vector can be empty or can have the same length
   * as channel_names
   * @param cost estimated relative cost of the function, default is 1,
   * can be changed to influence thread job distribution
   */
  ImageFunctionData(ImagePixelFunc ptr,
                    std::vector<std::string> channel_names,
                    const std::vector<real_t>& desired_max, real_t cost = 1.0)
      : mode(ImageMode::PIXEL_FUNC), channel_names(std::move(channel_names)),
        desired_max(desired_max), cost(cost) {
    this->ptr.pixel = ptr;
  }

  /**
   * Defines a function that works per image
   * @param ptr function pointer
   * @param channel_names which channels out of a channel collection to use
   */
  ImageFunctionData(WholeImageFunc ptr,
                    std::vector<std::string> channel_names)
      : mode(ImageMode::IMAGE_FUNC), channel_names(std::move(channel_names)) {
    this->ptr.whole = ptr;
  }

  ImageFunc ptr;
  ImageMode mode;
  std::vector<std::string> channel_names;
  std::vector<real_t> desired_max;
  real_t cost = 1;
};

/**
 * Structure for describing an image configuration, including function
 * specification and computed channel collection
 * @tparam real_t type representing real numbers
 * @tparam counter_t type for integer counter, for longer renders 16bit may not be enough
 */
template<class real_t = float, class counter_t = uint16_t>
struct ImageOutputData {
  ImageOutputData(const ImageFunctionData<real_t>& func,
                  NebulabrotChannelCollection<counter_t>* channels)
      : func(func), channels(channels) {}

  real_t getCost() const {
    return channels->getWidth() * channels->getHeight() * func.cost;
  }

  ImageFunctionData<real_t> func;
  NebulabrotChannelCollection<counter_t>* channels;
};

/**
 * Structure describing image rendering layer with its configuration
 * @tparam real_t type representing real numbers
 * @tparam counter_t type for integer counter, for longer renders 16bit may not be enough
 */
template<class real_t = float, class counter_t = uint16_t>
struct ImageRenderChannel {
  ImageRenderChannel(const ImageOutputData<real_t>& output_data,
                     std::string filename)
      : cost(output_data.getCost()), filename(std::move(filename)),
        output_data(output_data) {}

  real_t cost;
  std::string filename;
  ImageOutputData<real_t, counter_t> output_data;
  ImageColorBuffer* buf = nullptr;
  bool failed = false;
  size_t unfinished_jobs = 0;
  size_t threads_on_channel = 0;
  std::vector<std::pair<size_t, size_t>> render_jobs;

  inline bool operator<(const ImageRenderChannel& other) const {
    if (output_data.func.mode != other.output_data.func.mode) {
      return output_data.func.mode < other.output_data.func.mode;
    }
    return cost < other.cost;
  }
};

/**
 * Structure describing piece of work for image coloring threads
 * @tparam real_t type representing real numbers
 */
template<class real_t = float>
struct ImageJobData {
  inline static auto defaultFunc = [](real_t*) {
    return (uint32_t) 0;
  };

  ImageJobData()
      : output_data(
      ImageFunctionData<real_t>(defaultFunc,
                                {}, {}), nullptr) {}

  ImageOutputData<real_t> output_data;
  size_t start_index = 0;
  size_t end_index = 0;
  size_t num_image = 0;
};

/**
 * Main image rendering class, to use, add some channels/layers and execute
 * @tparam real_t type representing real numbers
 * @tparam counter_t type for integer counter, for longer renders 16bit may not be enough
 */
template<class real_t = float, class counter_t = uint16_t>
class ImageRenderingManager {
public:
  explicit ImageRenderingManager(size_t num_threads)
      : num_threads(num_threads) {}

  /**
   * Adds an output image to save
   * @param filename path of the image, without extension (only saving as png)
   * @param image_data description of image to save
   */
  void add(const std::string& filename,
           const ImageOutputData<real_t>& image_data) {
    auto insert_it = images.begin();
    for (auto it = insert_it; it != images.end(); ++it) {
      int comp_result = filename.compare(it->filename);
      if (comp_result == 0) {
        throw std::invalid_argument("Error while adding image to rendering manager: name conflict (" + filename + ")");
      } else if (comp_result < 0) {
        insert_it = it + 1;
      }
    }
    images.emplace(insert_it, image_data, filename);
  }

  /**
   * Does the image rendering in multiple threads, returns synchronously
   */
  void execute() {
    std::lock_guard<std::mutex> lock(execute_mutex);
    std::vector<ImageColorBuffer> image_buffers;
    if (images.empty()) {
      return;
    }
    render_start = std::chrono::high_resolution_clock::now();

    std::sort(images.begin(), images.end());
    real_t total_cost = 0.0;
    for (auto& im : images) {
      total_cost += im.cost;
      image_buffers.emplace_back(im.output_data.channels->getWidth(), im.output_data.channels->getHeight());
    }
    size_t approx_num_jobs = num_threads * 3 + static_cast<size_t>(std::log2(total_cost));

    jobs_total = 0;
    jobs_finished = 0;
    last_notification_elapsed = 0;
    size_t num = 0;
    for (auto& im : images) {
      std::cout << "Maximum channel values for " << im.filename << ":\n";
      for (auto& p : im.output_data.channels->channels) {
        std::cout << p.second.getMaxValue() << "\n";
      }
      size_t pixel_count = im.output_data.channels->getWidth() * im.output_data.channels->getHeight();
      im.buf = &image_buffers[num];
      num++;
      if (im.output_data.func.mode == ImageMode::IMAGE_FUNC) {
        im.render_jobs.emplace_back(0, pixel_count);
        im.unfinished_jobs = 1;
        jobs_total++;
      } else {
        size_t ch_jobs = std::max((size_t) 1,
                                  (size_t) (im.cost / total_cost * approx_num_jobs));
        size_t pixels_per_job_base = pixel_count / ch_jobs;
        size_t pixels_per_job_rem = pixel_count % ch_jobs;
        im.render_jobs.resize(ch_jobs);
        im.unfinished_jobs = ch_jobs;
        im.threads_on_channel = 0;
        size_t temp1 = 0;
        size_t temp2 = 0;
        for (size_t i = 0; i < ch_jobs; ++i) {
          if (i < pixels_per_job_rem) {
            temp2 += pixels_per_job_base + 1;
            im.render_jobs[i] = std::pair<size_t, size_t>(temp1, temp2);
            temp1 = temp2;
          } else {
            temp2 += pixels_per_job_base;
            im.render_jobs[i] = std::pair<size_t, size_t>(temp1, temp2);
            temp1 = temp2;
          }
        }
        jobs_total += ch_jobs;
      }
    }
    std::vector<std::thread> threads;
    size_t temp_channel_num = images.size() - 1;
    for (size_t i = 0; i < num_threads; ++i) {
      images[temp_channel_num].threads_on_channel++;
      threads.emplace_back(&ImageRenderingManager::threadFunction, this, temp_channel_num);
      if (temp_channel_num == 0) {
        temp_channel_num = images.size() - 1;
      } else {
        temp_channel_num--;
      }
    }
    for (size_t i = 0; i < num_threads; ++i) {
      threads[i].join();
    }
    double time = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - render_start).count();
    std::cout << "Saving images ended in " << time << std::endl;
  }


private:
  void notifyJobCompletion(size_t image_id) {
    notify_mutex.lock();
    images[image_id].unfinished_jobs--;
    jobs_finished++;
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - render_start).count();
    int elapsed_int = static_cast<int>(elapsed);
    if (elapsed_int != last_notification_elapsed) {
      last_notification_elapsed = elapsed_int;
      if (jobs_finished < jobs_total) {
        double approx_job_proggress = jobs_finished + std::min(num_threads, jobs_total - jobs_finished) / 3.0;
        double estimated = elapsed * (jobs_total - approx_job_proggress) / approx_job_proggress;
        std::cout << "(" << jobs_finished << "/" << jobs_total << ") Elapsed time: " << elapsed
                  << ", estimated remaining time: " << estimated << "\n";
      }
    }
    if (images[image_id].unfinished_jobs == 0 && !images[image_id].failed) {
      notify_mutex.unlock();
      images[image_id].buf->saveFile(images[image_id].filename);
    } else {
      notify_mutex.unlock();
    }
  }

  void failImage(size_t image_id) {
    std::lock_guard<std::mutex> lock(job_getter_mutex);
    std::lock_guard<std::mutex> lock2(notify_mutex);
    images[image_id].failed = true;
    images[image_id].render_jobs.clear();
  }

  void threadFunction(size_t start_image) {
    size_t previous_image =
        NebulabrotRenderingManager<real_t, counter_t>::NO_CHANNEL;
    while (true) {
      ImageJobData job = getAJob(start_image);
      if (job.start_index == job.end_index) {
        return;
      }
      start_image = job.num_image;
      if (previous_image != start_image) {
        if (previous_image !=
            NebulabrotRenderingManager<real_t, counter_t>::NO_CHANNEL) {
        }
      }
      doJob(job, start_image);
      previous_image = start_image;
      notifyJobCompletion(start_image);
    }
  }

  ImageJobData<real_t> getAJob(size_t preferred_image) {
    std::lock_guard<std::mutex> lock(job_getter_mutex);
    ImageJobData result;
    bool found = false;
    size_t images_size = images.size();
    for (size_t i = 0; i < images_size; ++i) {
      size_t vec_size = images[preferred_image].render_jobs.size();
      if (vec_size > 0) {
        auto p = images[preferred_image].render_jobs[vec_size - 1];
        result.start_index = p.first;
        result.end_index = p.second;
        images[preferred_image].render_jobs.pop_back();
        found = true;
        break;
      }
      if (preferred_image > 0) {
        preferred_image--;
      } else {
        preferred_image = images_size - 1;
      }
    }
    if (!found) {
      return result;
    }
    result.output_data = images[preferred_image].output_data;
    result.num_image = preferred_image;
    return result;
  }

  void doJob(const ImageJobData<real_t>& job, size_t image_num) {
    size_t num_channels = job.output_data.func.channel_names.size();
    std::vector<counter_t*> input_channels;
    std::vector<counter_t> maximum_values;
    std::vector<size_t> completed_iterations;
    std::vector<real_t> desired_max = job.output_data.func.desired_max;
    input_channels.reserve(num_channels);
    maximum_values.reserve(num_channels);
    completed_iterations.reserve(num_channels);
    if (desired_max.empty()) {
      desired_max.resize(job.output_data.func.channel_names.size());
    } else if (desired_max.size() != job.output_data.func.channel_names.size()) {
      if (!images[image_num].failed) {
        failImage(image_num);
        std::cout << "Error while saving image " + images[image_num].filename
                     + ": desired_max vector has wrong size\n";
      }
      return;
    }
    for (auto& ch_name : job.output_data.func.channel_names) {
      auto it = job.output_data.channels->channels.find(ch_name);
      if (it == job.output_data.channels->channels.end()) {
        if (!images[image_num].failed) {
          failImage(image_num);
          std::cout << "Error while saving image " + images[image_num].filename
                       + ": no channel named " + ch_name + "\n";
        }
        return;
      } else {
        input_channels.push_back(it->second.getData() + job.start_index);
        counter_t max_value = it->second.getMaxValue();
        if (max_value == 0) {
          it->second.updateMaxValue();
          max_value = it->second.getMaxValue();
        }
        maximum_values.push_back(max_value);
        completed_iterations.push_back(it->second.getCompletedIterations());
      }
    }
    if (job.output_data.func.mode == ImageMode::IMAGE_FUNC) {
      job.output_data.func.ptr.whole(job.end_index - job.start_index, input_channels.data(), maximum_values.data(),
                                     images[image_num].buf->getData() + job.start_index);
    } else {
      std::vector<real_t> current_values(num_channels);
      std::vector<real_t> multiplier(num_channels);
      uint32_t* output = images[image_num].buf->getData() + job.start_index;

      for (size_t j = 0; j < num_channels; ++j) {
        if (desired_max[j] <= 0.0) {
          multiplier[j] = 1;
        } else {
          multiplier[j] = desired_max[j] * completed_iterations[j] / maximum_values[j];
        }
      }

      size_t len = job.end_index - job.start_index;
      for (size_t i = 0; i < len; ++i) {
        for (size_t j = 0; j < num_channels; ++j) {
          current_values[j] = multiplier[j] * input_channels[j][i] / maximum_values[j];
        }
        output[i] = job.output_data.func.ptr.pixel(current_values.data());
      }
    }
  }

  std::vector<ImageRenderChannel<real_t>> images;
  std::mutex execute_mutex;
  std::mutex job_getter_mutex;
  std::mutex notify_mutex;
  std::chrono::time_point<std::chrono::high_resolution_clock> render_start;
  int last_notification_elapsed = 0;
  size_t jobs_total = 0;
  size_t jobs_finished = 0;
  size_t num_threads;
};

}

#endif
