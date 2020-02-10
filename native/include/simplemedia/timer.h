#ifndef GURUM_TIMER_H_
#define GURUM_TIMER_H_

#include <cstdint>
#include <chrono>

namespace gurum {

class Timer {
public:
  Timer();
  void wait(int64_t period);
  void update();

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> target_time_;
};

}

#endif // GURUM_TIMER_H_
