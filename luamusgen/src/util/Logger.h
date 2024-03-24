//
// Created by rutio on 17.01.19.
//

#ifndef LUAMUSGEN_LOGGER_H
#define LUAMUSGEN_LOGGER_H

#include <string>
#include <mutex>

namespace logger {
  extern std::mutex print_mutex;
};

std::string demangle(const char* mangledName);

void logError(const char* funcName, const char* fmt, ...);
void logWarning(const char* funcName, const char* fmt, ...);
void logInfo(const char* funcName, const char* fmt, ...);
void logDebug(const char* funcName, const char* fmt, ...);

//void logDebug(const std::string& fmt, ...);

#define CLASS_NAME(obj) demangle(typeid(obj).name())
#define THIS_CLASS_NAME CLASS_NAME(*this)
#define CLASS_AND_FUNCTION (THIS_CLASS_NAME + "::" + __FUNCTION__).c_str()

#define logErrorF(msg, ...) logError(__FUNCTION__, msg, ##__VA_ARGS__);
#define logWarningF(msg, ...) logWarning(__FUNCTION__, msg, ##__VA_ARGS__);
#define logInfoF(msg, ...) logInfo(__FUNCTION__, msg, ##__VA_ARGS__);
#define logDebugF(msg, ...) logDebug(__FUNCTION__, msg, ##__VA_ARGS__);

#define logErrorC(msg, ...) logError(CLASS_AND_FUNCTION, msg, ##__VA_ARGS__);
#define logWarningC(msg, ...) logWarning(CLASS_AND_FUNCTION, msg, ##__VA_ARGS__);
#define logInfoC(msg, ...) logInfo(CLASS_AND_FUNCTION, msg, ##__VA_ARGS__);
#define logDebugC(msg, ...) logDebug(CLASS_AND_FUNCTION, msg, ##__VA_ARGS__);

#ifdef PROCESS_DEBUG

#define debugProcess(msg, ...) logDebug(msg, ##__VA_ARGS__);
#define debugProcessF(msg, ...) logDebugF(msg, ##__VA_ARGS__);
#define debugProcessC(msg, ...) logDebugC(msg, ##__VA_ARGS__);

#else

#define debugProcess(msg, ...) ;
#define debugProcessF(msg, ...) ;
#define debugProcessC(msg, ...) ;

#endif


/* LUA-CALLABLE FUNCTIONS */
extern "C" {

int getWarningCount();
int getErrorCount();
void setLoggerLevel(int level);

}


#endif //LUAMUSGEN_LOGGER_H
