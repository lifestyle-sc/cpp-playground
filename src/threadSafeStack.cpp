#include <memory>
#include <mutex>
#include <threadSafeStack.h>

namespace prep {
template <typename T> ThreadSafeStack<T>::ThreadSafeStack(const ThreadSafeStack &other) {
    std::lock_guard lock_guard{d_stack_mutex};
    d_stack = other.d_stack;
}

template <typename T> void ThreadSafeStack<T>::push(T new_value) {
    std::lock_guard lock_guard{d_stack_mutex};
    d_stack.push(std::move(new_value));
}

template <typename T> void ThreadSafeStack<T>::pop(T &value) {
    std::lock_guard lock_guard{d_stack_mutex};
    if (d_stack.empty()) {
        throw empty_stack();
    }
    value = std::move(d_stack.top());
    d_stack.pop();
}

template <typename T> std::shared_ptr<T> ThreadSafeStack<T>::pop() {
    std::lock_guard lock_guard{d_stack_mutex};
    if (d_stack.empty()) {
        throw empty_stack();
    }
    std::shared_ptr<T> const res(std::make_shared(std::move(d_stack.top())));
    return res;
}

template <typename T> bool ThreadSafeStack<T>::empty() const {
    std::lock_guard lock_guard{d_stack_mutex};
    return d_stack.empty();
}
} // namespace prep