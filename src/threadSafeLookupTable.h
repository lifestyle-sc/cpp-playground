#ifndef INCLUDED_THREAD_SAFE_LOOKUP_TABLE
#define INCLUDED_THREAD_SAFE_LOOKUP_TABLE

#include <algorithm>
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>

namespace prep {
template <typename Key, typename Value, typename Hash = std::hash<Key>>
class ThreadSafeLookupTable {
  private:
    class bucket_type {
      private:
        using bucket_value = std::pair<Key, Value>;
        using bucket_data = std::list<bucket_value>;
        using bucket_iterator = bucket_data::iterator;

        std::shared_mutex d_bucket_mutex;
        bucket_data d_data;

      private:
        bucket_iterator _find_entry_or(const Key &key) {
            return std::find_if(d_data.begin(), d_data.end(),
                                [key = &key](const bucket_value &bucket_value) {
                                    return bucket_value.first == key;
                                });
        }

      public:
        Value get_or_default(const Key &key, const Value &default_value) {
            std::shared_lock<std::shared_mutex> shared_mutex{d_bucket_mutex};
            bucket_iterator const entry_maybe = _find_entry_or(key);
            return entry_maybe == d_data.end() ? default_value : entry_maybe->second;
        }

        void add_or_update(const Key &key, const Value &value) {
            std::unique_lock<std::shared_mutex> unique_lock{d_bucket_mutex};
            bucket_iterator entry_maybe = _find_entry_or(key);
            if (entry_maybe == d_data.end()) {
                d_data.push_back(bucket_value(key, value));
            } else {
                entry_maybe->second = value;
            }
        }

        void remove(const Key &key) {
            std::unique_lock<std::shared_mutex> unique_lock{d_bucket_mutex};
            bucket_iterator entry_maybe = _find_entry_or(key);
            if (entry_maybe != d_data.end()) {
                d_data.erase(key);
            }
        }
    };

  private:
    Hash d_hasher;
    std::vector<std::unique_ptr<bucket_type>> d_buckets;

    bucket_type &_get_buckets(const Key &key) const;

  public:
    using key_type = Key;
    using value_type = Value;
    using hash_type = Hash;

  public:
    ThreadSafeLookupTable(std::size_t num_buckets = 19, const hash_type &hasher = Hash());

    ThreadSafeLookupTable(const ThreadSafeLookupTable &) = delete;
    ThreadSafeLookupTable &operator=(const ThreadSafeLookupTable &) = delete;

    value_type get_or_default(const key_type &key, const value_type &value = value_type());

    void add_or_update(const key_type &key, const value_type &value);

    void remove(const key_type &key);

    std::map<key_type, value_type> to_map() const;
};
} // namespace prep

#endif