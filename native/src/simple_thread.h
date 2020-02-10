#ifndef GURUM_BASE_SIMPLE_THREAD_H_
#define GURUM_BASE_SIMPLE_THREAD_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>

namespace base {

class SimpleThread {

using Task=std::function<void()>;

public:
  virtual ~SimpleThread();

  int Init(int numof_thread=1);
  int Start();
  void Stop();

  void SetTimeout(int sec){timeout_ = sec;};

  int PostTask(Task task);

  Task GetTask();

  void Run();

  static std::unique_ptr<base::SimpleThread> CreateThread();

private:

private:
  std::list<Task> task_queue_;

  std::list<std::thread> threads_;
  std::mutex lck_;
  std::condition_variable cond_;

  bool started_{false};
  bool stopped_{false};

  int numof_thread_{1};
  int timeout_{0};
};

} // namespace base

#endif // GURUM_BASE_SIMPLE_THREAD_H_