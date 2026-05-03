#ifndef INCLUDED_THREAD_SAFE_LIST
#define INCLUDED_THREAD_SAFE_LIST

#include <concepts>
#include <memory>
#include <mutex>
namespace prep {

template <typename Predicate, typename Node>
concept ListPredicate = std::predicate<Predicate, const Node &>;

template <typename T> class ThreadSafeList {
    struct node {
        std::mutex d_node_mutex;
        std::shared_ptr<T> d_data;
        std::unique_ptr<node> d_next;

        node() : d_next() {}
        node(const T &data) : d_data(std::make_shared<T>(data)) {}
    };

    node d_head;

  public:
    ThreadSafeList() = default;
    ~ThreadSafeList();

  public:
    void push_front(const T &data);

    template <typename Predicate>
        requires ListPredicate<Predicate, node>
    void for_each(Predicate func);

    template <typename Predicate>
        requires ListPredicate<Predicate, node>
    std::shared_ptr<T> find_first_if(Predicate func);

    template <typename Predicate>
        requires ListPredicate<Predicate, node>
    void remove_if(Predicate func);
};
} // namespace prep

#endif