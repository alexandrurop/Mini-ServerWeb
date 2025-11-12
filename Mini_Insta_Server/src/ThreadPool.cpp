#include "./../include/ThreadPool.h"
using namespace std;

ThreadPool::ThreadPool(size_t n) {
    if (n == 0) n = 2;
    workers_.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                function<void()> job;
                {
                    unique_lock<mutex> lk(m_);
                    cv_.wait(lk, [this] { return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty()) return;
                    job = move(tasks_.front());
                    tasks_.pop();
                }
                job();
            }
            });
    }
}

ThreadPool::~ThreadPool() {
    {
        lock_guard<mutex> lk(m_);
        stop_ = true;
    }
    cv_.notify_all();
    for (auto& t : workers_) t.join();
}

void ThreadPool::submit(function<void()> job) {
    {
        lock_guard<mutex> lk(m_);
        tasks_.push(move(job));
    }
    cv_.notify_one();
}
