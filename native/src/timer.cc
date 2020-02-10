#include "timer.h"
#include <algorithm>
#include <thread>

namespace gurum {

Timer::Timer() :
  target_time_{std::chrono::high_resolution_clock::now()} {
}

void Timer::wait(const int64_t period) {
  target_time_ += std::chrono::microseconds{period};

  const auto lag =
    std::chrono::duration_cast<std::chrono::microseconds>(
      target_time_ - std::chrono::high_resolution_clock::now());

  std::this_thread::sleep_for(lag);
}

void Timer::update() {
  target_time_ = std::chrono::high_resolution_clock::now();
}

}