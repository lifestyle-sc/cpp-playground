#ifndef INCLUDED_THREAD_SAFE_STACK
#define INCLUDED_THREAD_SAFE_STACK

#include <exception>
#include <memory>
#include <mutex>
#include <stack>

namespace prep {
struct empty_stack : public std::exception {
    const char *what() const throw() { return "The stack is empty!"; }
};

template <typename T> class ThreadSafeStack {
    std::stack<T> d_stack;
    mutable std::mutex d_stack_mutex;

  public:
    ThreadSafeStack() = default;

    ThreadSafeStack(const ThreadSafeStack &other);

    ThreadSafeStack &operator=(const ThreadSafeStack &other) = delete;

    void push(T new_value);

    std::shared_ptr<T> pop();

    void pop(T &value);

    bool empty() const;
};
} // namespace prep

#endif