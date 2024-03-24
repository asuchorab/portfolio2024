//
// Created by rutio on 17.01.19.
//

#include "Logger.h"
#include <sstream>
#include <cstdarg>
#include <regex>
#include <iostream>

thread_local int error_count = 0;
thread_local int warning_count = 0;
thread_local int logger_level = 4;

#ifdef __GNUG__

#include <cxxabi.h>
#include <stdlib.h>

std::string demangle(const char* mangledName) {
  std::string result;
  std::size_t len = 0;
  int status = 0;
  char* ptr = __cxxabiv1::__cxa_demangle(mangledName, nullptr, &len, &status);
  if(status == 0) result = ptr; // hope that this won't throw
  else result = "demangle error";
  ::free(ptr);
  return result;
}

#else

std::string demangle(const char* mangledName) {
  return mangledName;
}

#endif

std::mutex logger::print_mutex;

void logError(const char* funcName, const char* fmt, ...) {
  error_count++;
  if (logger_level < 1) {
    return;
  }

  va_list argptr;
  std::stringstream ss;
  ss << "[ERROR] " << std::regex_replace(funcName, std::regex("%"), "%%") << ": " << fmt << '\n';
  std::string final_fmt = ss.str();
  va_start(argptr, fmt);
  {
    std::lock_guard<std::mutex> lock(logger::print_mutex);
    vfprintf(stderr, final_fmt.c_str(), argptr);
  }
  va_end(argptr);
}

void logWarning(const char* funcName, const char* fmt, ...) {
  warning_count++;
  if (logger_level < 2) {
    return;
  }

  va_list argptr;
  std::stringstream ss;
  ss << "[WARN ] " << std::regex_replace(funcName, std::regex("%"), "%%") << ": " << fmt << '\n';
  std::string final_fmt = ss.str();
  va_start(argptr, fmt);
  {
    std::lock_guard<std::mutex> lock(logger::print_mutex);
    vfprintf(stderr, final_fmt.c_str(), argptr);
  }
  va_end(argptr);
}

void logInfo(const char* funcName, const char* fmt, ...) {
  if (logger_level < 3) {
    return;
  }

  va_list argptr;
  std::stringstream ss;
  ss << "[INFO ] " << std::regex_replace(funcName, std::regex("%"), "%%") << ": " << fmt << '\n';
  std::string final_fmt = ss.str();
  va_start(argptr, fmt);
  {
    std::lock_guard<std::mutex> lock(logger::print_mutex);
    vfprintf(stdout, final_fmt.c_str(), argptr);
  }
  va_end(argptr);
}

void logDebug(const char* funcName, const char* fmt, ...) {
  if (logger_level < 4) {
    return;
  }

  va_list argptr;
  std::stringstream ss;
  ss << "[DEBUG] " << std::regex_replace(funcName, std::regex("%"), "%%") << ": " << fmt << '\n';
  std::string final_fmt = ss.str();
  va_start(argptr, fmt);
  {
    std::lock_guard<std::mutex> lock(logger::print_mutex);
    vfprintf(stdout, final_fmt.c_str(), argptr);
  }
  va_end(argptr);
}

int getWarningCount() {
  return warning_count;
}

int getErrorCount() {
  return error_count;
}

void setLoggerLevel(int level) {
  if (level < 0 || level > 4) {
    logErrorF("expected value from 0 to 4 [none, error, warning, info, debug]");
    return;
  }
  logger_level = level;
}
