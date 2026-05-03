#include <threadSafeList.h>

#include <memory>
#include <mutex>
#include <utility>

namespace prep {
template <typename T> ThreadSafeList<T>::~ThreadSafeList() {
    remove_if([&](const node &) { return true; });
}

template <typename T> void ThreadSafeList<T>::push_front(const T &data) {
    auto new_node = std::make_unique<T>(data);
    std::lock_guard<std::mutex> head_lock{d_head.d_node_mutex};
    new_node->next = std::move(d_head.d_next);
    d_head.d_next = std::move(new_node);
}

template <typename T>
template <typename Predicate>
    requires ListPredicate<Predicate, typename ThreadSafeList<T>::node>
void ThreadSafeList<T>::for_each(Predicate func) {
    node *current = &d_head;
    std::unique_lock<std::mutex> curr_ulk{current->d_node_mutex};
    while (node *next = current->d_next.get()) {
        std::unique_lock<std::mutex> next_ulk{next->d_node_mutex};
        curr_ulk.unlock();
        func(*next->d_data);
        current = next;
        curr_ulk = std::move(next_ulk);
    }
}

template <typename T>
template <typename Predicate>
    requires ListPredicate<Predicate, typename ThreadSafeList<T>::node>
std::shared_ptr<T> ThreadSafeList<T>::find_first_if(Predicate func) {
    node *current = &d_head;
    std::unique_lock<std::mutex> curr_ulk{current->d_node_mutex};
    while (node *next = current->d_next.get()) {
        std::unique_lock<std::mutex> next_ulk{next->d_node_mutex};
        curr_ulk.unlock();
        if (func(*next->d_data)) {
            next_ulk.unlock();
            return next->d_data;
        } else {
            current = next;
            curr_ulk = std::move(next_ulk);
        }
    }
    return nullptr;
}

template <typename T>
template <typename Predicate>
    requires ListPredicate<Predicate, typename ThreadSafeList<T>::node>
void ThreadSafeList<T>::remove_if(Predicate func) {
    node *current = &d_head;
    std::unique_lock<std::mutex> curr_ulk{current->d_node_mutex};
    while (node *next = current->d_next.get()) {
        std::unique_lock<std::mutex> next_ulk{next->d_node_mutex};
        if (func(*next->d_data)) {
            auto old_next = std::move(current->d_next);
            current->d_next = std::move(next->d_next);
            next_ulk.unlock();
        } else {
            curr_ulk.unlock();
            current = next;
            curr_ulk = std::move(next_ulk);
        }
    }
}
} // namespace prep