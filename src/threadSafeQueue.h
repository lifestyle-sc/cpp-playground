#ifndef INCLUDED_THREAD_SAFE_QUEUE
#define INCLUDED_THREAD_SAFE_QUEUE

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace prep {
template <typename T> class ThreadSafeQueue {
    mutable std::mutex d_queue_mutex;
    std::queue<T> d_queue;
    std::condition_variable d_queue_cv;

  public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue &other);
    ThreadSafeQueue &operator=(const ThreadSafeQueue &other) = delete;
    void push(T value);
    bool empty() const;

    void wait_and_pop(T &value);
    std::shared_ptr<T> wait_and_pop();

    bool try_pop(T &value);
    std::shared_ptr<T> try_pop();
};

} // namespace prep

#endif