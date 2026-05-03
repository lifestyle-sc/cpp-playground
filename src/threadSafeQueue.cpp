#include <memory>
#include <mutex>
#include <threadSafeQueue.h>

namespace prep {
template <typename T> ThreadSafeQueue<T>::ThreadSafeQueue(const ThreadSafeQueue &other) {
    std::lock_guard lock_guard{d_queue_mutex};
    d_queue = other.d_queue;
}

template <typename T> void ThreadSafeQueue<T>::push(T value) {
    {
        std::lock_guard lock_guard{d_queue_mutex};
        d_queue.push(std::move(value));
    }
    d_queue_cv.notify_one();
}

template <typename T> bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard lock_guard{d_queue_mutex};
    return d_queue.empty();
}

template <typename T> void ThreadSafeQueue<T>::wait_and_pop(T &value) {
    std::unique_lock unique_lock{d_queue_mutex};
    d_queue_cv.wait(unique_lock, [this] { return !d_queue.empty(); });

    value = std::move(d_queue.front());
    d_queue.pop();
}

template <typename T> std::shared_ptr<T> ThreadSafeQueue<T>::wait_and_pop() {
    std::unique_lock unique_lock{d_queue_mutex};
    d_queue_cv.wait(unique_lock, [d_queue = &d_queue] { return !d_queue.empty(); });

    std::shared_ptr<T> const res(std::make_shared(std::move(d_queue.front())));
    d_queue.pop();
    return res;
}

template <typename T> bool ThreadSafeQueue<T>::try_pop(T &value) {
    std::lock_guard lock_guard{d_queue_mutex};
    if (d_queue.empty()) {
        return false;
    }

    value = std::move(d_queue.front());
    d_queue.pop();
    return true;
}

template <typename T> std::shared_ptr<T> ThreadSafeQueue<T>::try_pop() {
    std::lock_guard lock_guard{d_queue_mutex};
    if (d_queue.empty()) {
        return nullptr;
    }

    std::shared_ptr<T> const res(std::make_shared(std::move(d_queue.front())));
    d_queue.pop();
    return res;
}
} // namespace prep