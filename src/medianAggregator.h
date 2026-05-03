#ifndef INCLUDED_MEDIAN_AGGREGATOR
#define INCLUDED_MEDIAN_AGGREGATOR

/*
Design and implement a C++ class that consumes timestamped records form multiple ordered streams
and emits the median value for each timestamp once that timestamp is known to be complete.

A timestamp t is complete once every stream has either:
- emitted some record with timestamp greater than t, or
- been closed

When t becomes complete, emit the median of all values from the records with timestamp t across all
streams.

Assumptions:
- 0 <= stream_id < N
- N is known at construction time
- A stream may emit zero, one, or many recordsfor a timestamp
- Some streams may be have no record for a given timestamp
- Median for an even number of values is defined as the average of the two middle values.
*/

#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <vector>
namespace prep {

struct Record {
    std::int64_t timestamp;
    double value;
};

enum class MedianAggregatorError : int { SUCCESS = 0, LOW_TIMESTAMP = 1 };

class MedianAggregator {
    // Aliases
    using Timestamp = std::int64_t;
    using Value = double;
    using StreamId = std::size_t;
    using StreamBucket = std::vector<Value>;

    // Data Members
    std::map<Timestamp, StreamBucket> d_pending_by_timestamp;
    std::vector<Timestamp> d_latest_stream_timestamp;
    std::vector<Value> d_emitted_records;
    std::mutex d_mutex;

  private:
    double _compute_median_value(std::vector<Value> &values) noexcept;
    void _collect_completed_timestamps(std::vector<StreamBucket> &stream_buckets,
                                       Timestamp completed_bound) noexcept;
    void _emit_completed_timestamps(std::vector<StreamBucket> &stream_buckets) noexcept;

  public:
    MedianAggregator(std::size_t stream_size);

  public:
    MedianAggregatorError push(StreamId stream_id, Record record);
    void close(StreamId stream_id);
    void advance_watermark(StreamId stream_id, Timestamp watermark) noexcept;
    const std::vector<Value> &emit() const noexcept;
};

} // namespace prep

#endif