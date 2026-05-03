#include <memory>
#include <mutex>
#include <threadSafeQueueLL.h>
#include <utility>

namespace prep {
template <typename T>
ThreadSafeQueueLL<T>::ThreadSafeQueueLL() : d_head(new node{}), d_tail(d_head.get()) {}

template <typename T> auto ThreadSafeQueueLL<T>::_get_tail_node() -> node * {
    std::lock_guard<std::mutex> tail_lock_guard{d_tail_mutex};
    return d_tail;
}

template <typename T> auto ThreadSafeQueueLL<T>::_pop_head() -> std::unique_ptr<node> {
    auto old_head = std::move(d_head);
    d_head = old_head->d_next;
    return old_head;
}

template <typename T> std::unique_lock<std::mutex> ThreadSafeQueueLL<T>::_wait_for_data() {
    std::unique_lock<std::mutex> head_ul{d_head_mutex};
    d_empty_cv.wait(head_ul, [&] { return d_head.get() != _get_tail_node(); });
    return head_ul;
}

template <typename T> auto ThreadSafeQueueLL<T>::_wait_pop_head() -> std::unique_ptr<node> {
    std::unique_lock<std::mutex> head_ul{_wait_for_data()};
    return _pop_head();
}

template <typename T> auto ThreadSafeQueueLL<T>::_wait_pop_head(T &value) -> std::unique_ptr<node> {
    std::unique_lock<std::mutex> head_ul{_wait_for_data()};
    value = std::move(*d_head->d_data);
    return _pop_head();
}

template <typename T> auto ThreadSafeQueueLL<T>::_try_pop_head() -> std::unique_ptr<node> {
    std::lock_guard<std::mutex> head_lock_guard{d_head_mutex};
    if (d_head.get() == _get_tail_node()) {
        return std::unique_ptr<T>{};
    }
    return _pop_head();
}

template <typename T> auto ThreadSafeQueueLL<T>::_try_pop_head(T &value) -> std::unique_ptr<node> {
    std::lock_guard<std::mutex> head_lock_guard{d_head_mutex};
    if (d_head.get() == _get_tail_node()) {
        return std::unique_ptr<T>{};
    }
    value = std::move(*(d_head->d_data));
    return _pop_head();
}

template <typename T> void ThreadSafeQueueLL<T>::push(T value) {
    auto new_data = std::make_shared(std::move(value));
    std::unique_ptr<node> dummy_tail(new node{});
    {
        std::lock_guard<std::mutex> tail_lock_guard{d_tail_mutex};
        d_tail->d_data = new_data;
        node *const new_tail = dummy_tail.get();
        d_tail->d_next = std::move(dummy_tail);
        d_tail = new_tail;
    }
    d_empty_cv.notify_one();
}

template <typename T> bool ThreadSafeQueueLL<T>::empty() const {
    std::lock_guard<std::mutex> head_lock_guard{d_head_mutex};
    return d_head.get() == _get_tail_node();
}

template <typename T> void ThreadSafeQueueLL<T>::wait_and_pop(T &value) {
    auto const old_head = _wait_pop_head(value);
}

template <typename T> std::shared_ptr<T> ThreadSafeQueueLL<T>::wait_and_pop() {
    auto const old_head = _wait_pop_head();
    return old_head->d_data;
}

template <typename T> bool ThreadSafeQueueLL<T>::try_pop(T &value) {
    auto const old_head = _try_pop_head(value);
    return old_head;
}

template <typename T> std::shared_ptr<T> ThreadSafeQueueLL<T>::try_pop() {
    auto const old_head = _try_pop_head();
    return old_head ? old_head->d_data : std::shared_ptr<T>{};
}
} // namespace prep