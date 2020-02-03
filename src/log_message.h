#ifndef BASE_LOGGING_LOG_MESSAGE_H_
#define BASE_LOGGING_LOG_MESSAGE_H_

// #define USE_SYSLOG
#undef USE_SYSLOG

#if !defined(USE_GLOG)

#if defined(LOG)
#undef LOG
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sstream>
#include <ostream>
#include <string.h>
#include <pthread.h>
#include <chrono>
#include <iostream>


#if defined(USE_SYSLOG)
#include <syslog.h>
#endif

typedef int LogSeverity;
#if defined(USE_SYSLOG)
const LogSeverity VERBOSE = -1;
const LogSeverity INFO = LOG_INFO;
const LogSeverity WARNING = LOG_WARNING;
const LogSeverity ERROR = LOG_ERR;
const LogSeverity FATAL = LOG_ERR;
#else
const LogSeverity VERBOSE = -1;
const LogSeverity INFO = 1;
const LogSeverity WARNING = 2;
const LogSeverity ERROR = 3;
const LogSeverity FATAL = 4;
#endif

namespace base {

namespace logging {

class LogMessage {
public:
  LogMessage(LogSeverity severity, const std::string &file, const std::string &func, int line)
  : severity_(severity), file_(file), func_(func), line_(line) {
    static auto start = std::chrono::high_resolution_clock::now();
    stream() << "{" << severity_ << "}" 
        << "[" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() << "]"
        << "[" << std::hex <<  (uint64_t) pthread_self() << "]" << std::dec  << file_ << ":" <<line_ << "] ";
  }

  ~LogMessage() {
#if defined(USE_SYSLOG)
    syslog(severity_, "%s",stream_.str().c_str());
#else
    std::cerr << stream_.str() << std::endl;
#endif
  }

  std::ostream &stream() {return stream_;}

private:
  std::ostringstream stream_;
  LogSeverity severity_;
  const std::string file_;
  const std::string func_;
  const unsigned int line_;
};

} // namespace logging

} // namespace base

#define LOG(verbose_level) ::base::logging::LogMessage(verbose_level, __FILE__, __func__, __LINE__).stream()


#else
#include <glog/logging.h>
#endif



#include <sstream>
#include <ostream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>


namespace {

// static std::string Hexdump(const uint8_t *data, size_t len) {
//   std::ostringstream stream;
//   stream << std::hex << std::setfill ('0') << std::setw (2);

//   for(int i=0; i < (int)len; i++) {

//     stream << (int) (0xFF & data[i]) << " ";

//     if(((i+1)%0x10)==0) stream << "\n";
//   }

//   stream << std::dec << "\n";


//   stream << " [";

//   for(int i=0; i < (int)len; i++) {
//     char tmp = '.';
//     if(((data[i] >= 'A') && (data[i] <= 'z')) || ((data[i] >= '0') && (data[i] <= '9')) || (data[i]==',')
//         || (data[i]=='\"') || (data[i]==':') || (data[i]=='{') || (data[i]=='}')) {
//       tmp = data[i];
//     }
//     stream << tmp << " ";

//     if(((i+1)%0x10)==0) stream << "\n";
//   }

//   stream << " ]";

//   return stream.str();
// }

}

#endif // BASE_LOGGING_LOG_MESSAGE_H_

