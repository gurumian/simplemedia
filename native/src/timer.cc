#include "timer.h"
#include <algorithm>
#include <thread>

namespace gurum {

Timer::Timer() :
  target_time_{std::chrono::high_resolution_clock::now()} {
}

void Timer::wait(const int64_t period) {
  target_time_ += std::chrono::microseconds{period};

  const auto diff =
    std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now() - target_time_);

  std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::microseconds{period} - diff));
}

void Timer::update() {
  target_time_ = std::chrono::high_resolution_clock::now();
}

}