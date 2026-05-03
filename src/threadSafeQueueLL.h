#ifndef INCLUDED_THREAD_SAFE_QUEUE_LL
#define INCLUDED_THREAD_SAFE_QUEUE_LL

#include <condition_variable>
#include <memory>
#include <mutex>

namespace prep {
template <typename T> class ThreadSafeQueueLL {
    struct node {
        std::shared_ptr<T> d_data;
        std::unique_ptr<node> d_next;
    };

    mutable std::mutex d_head_mutex;
    mutable std::mutex d_tail_mutex;
    std::unique_ptr<node> d_head;
    node *d_tail;
    std::condition_variable d_empty_cv;

  private:
    node *_get_tail_node();

    std::unique_ptr<node> _pop_head();

    std::unique_lock<std::mutex> _wait_for_data();

    std::unique_ptr<node> _wait_pop_head();

    std::unique_ptr<node> _wait_pop_head(T &value);

    std::unique_ptr<node> _try_pop_head();

    std::unique_ptr<node> _try_pop_head(T &value);

  public:
    ThreadSafeQueueLL();
    ThreadSafeQueueLL(const ThreadSafeQueueLL &other) = delete;
    ThreadSafeQueueLL &operator=(const ThreadSafeQueueLL &other) = delete;

    void push(T value);
    bool empty() const;

    void wait_and_pop(T &value);
    std::shared_ptr<T> wait_and_pop();

    bool try_pop(T &value);
    std::shared_ptr<T> try_pop();
};

} // namespace prep

#endif