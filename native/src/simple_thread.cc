#include "simple_thread.h"
#include <iostream>
#include <assert.h>
#include <chrono>
#include "log_message.h"

namespace base {

SimpleThread::~SimpleThread() {
  if(started_ && ! stopped_) {
    Stop();
  }
}

int SimpleThread::Init(int numof_thread) {
  numof_thread_ = numof_thread;
  return 0;
}

int SimpleThread::Start() {
  for(int i = 0; i < numof_thread_; i++) {
    threads_.push_back(std::thread([&](){
      while(!stopped_) Run();
    }));
  }
  started_ = true;
  return 0;
}

void SimpleThread::Stop() {
  for(auto &thread : threads_) {
    assert(thread.native_handle() != pthread_self());
    if(thread.native_handle() == pthread_self()) {
     LOG(WARNING) << __func__ << " This thread is not allowed to call this method" << std::endl;
      return;
    }
  }

  if(stopped_) return;

  stopped_ = true;
  cond_.notify_all();

  if(started_) {
    for(auto &thread : threads_) {
      thread.join();
    }
  }
}

void SimpleThread::Run() {
  auto task = GetTask();
  if(task) task();
}

int SimpleThread::PostTask(Task task) {
  std::lock_guard<std::mutex> lk(lck_);
  task_queue_.push_back(task);
  cond_.notify_one();
  return 0;
}

SimpleThread::Task SimpleThread::GetTask() {
  std::unique_lock<std::mutex> lk(lck_);

  for(;;) {
    if(! task_queue_.empty()) break;

    if(timeout_ > 0) {
      const auto sec = std::chrono::seconds(1);
      std::cv_status st = cond_.wait_for(lk, timeout_*sec);
      if(st == std::cv_status::timeout) {
        return nullptr;
      }
    }
    else {
      cond_.wait(lk);
    }

    if(stopped_) return nullptr;
  }

  auto task = task_queue_.front();
  task_queue_.pop_front();
  return task;
}

std::unique_ptr<base::SimpleThread> SimpleThread::CreateThread() {
  std::unique_ptr<base::SimpleThread> thread(new base::SimpleThread);
  thread->Init();
  thread->Start();
  return thread;
}

} // namespace base