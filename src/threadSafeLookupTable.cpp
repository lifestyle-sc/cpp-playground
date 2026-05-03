#include <threadSafeLookupTable.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <shared_mutex>

namespace prep {
template <typename Key, typename Value, typename Hash>
ThreadSafeLookupTable<Key, Value, Hash>::ThreadSafeLookupTable(std::size_t num_buckets,
                                                               const hash_type &hasher)
    : d_buckets(num_buckets), d_hasher(hasher) {
    std::generate(d_buckets.begin(), d_buckets.end(),
                  [] { return std::make_unique<bucket_type>(); });
}

template <typename Key, typename Value, typename Hash>
auto ThreadSafeLookupTable<Key, Value, Hash>::_get_buckets(const Key &key) const -> bucket_type & {
    const std::size_t key_index = d_hasher(key) % d_buckets.size();
    return *d_buckets[key_index];
}

template <typename Key, typename Value, typename Hash>
auto ThreadSafeLookupTable<Key, Value, Hash>::get_or_default(const key_type &key,
                                                             const value_type &value)
    -> value_type {
    return _get_buckets(key).get_or_default(key, value);
}

template <typename Key, typename Value, typename Hash>
void ThreadSafeLookupTable<Key, Value, Hash>::add_or_update(const key_type &key,
                                                            const value_type &value) {
    _get_buckets(key).add_or_update(key, value);
}

template <typename Key, typename Value, typename Hash>
void ThreadSafeLookupTable<Key, Value, Hash>::remove(const key_type &key) {
    _get_buckets(key).remove(key);
}

template <typename Key, typename Value, typename Hash>
auto ThreadSafeLookupTable<Key, Value, Hash>::to_map() const -> std::map<key_type, value_type> {
    std::vector<std::unique_lock<std::shared_mutex>> locks;
    for (auto &bucket : d_buckets) {
        locks.push_back(std::unique_lock<std::shared_mutex>{bucket.d_bucket_mutex});
    }
    std::map<key_type, value_type> res;
    for (auto &bucket : d_buckets) {
        for (auto &bucket_value : bucket) {
            res.insert(bucket_value);
        }
    }
    return res;
}

} // namespace prep