#ifndef INCLUDED_MEDIAN_AGGREGATOR_MPSC
#define INCLUDED_MEDIAN_AGGREGATOR_MPSC
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

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <medianAggregator.h>
#include <variant>

namespace prep {
struct PushEvent {
    std::size_t stream_id;
    Record record;
};

struct CloseEvent {
    std::size_t stream_id;
};

struct AdvanceEvent {
    std::size_t stream_id;
    int64_t watermark;
};

class MedianAggregatorMPSC {
    // Aliases
    using Event = std::variant<PushEvent, CloseEvent, AdvanceEvent>;
    using StreamId = std::size_t;
    using Timestamp = int64_t;
    using Value = double;
    using StreamBucket = std::vector<Value>;

    // Data Members
    std::map<Timestamp, StreamBucket> d_pending_by_timestamp;
    std::vector<Timestamp> d_latest_stream_timestamps;
    std::vector<Value> d_emitted_records;
    std::deque<Event> d_queue;
    std::mutex d_queue_mutex;
    std::condition_variable d_queue_cv;
    std::thread d_consumer;
    bool d_stopping;

    // Private Manipulators
    double _compute_median_value(std::vector<Value> &values) noexcept;
    void _collect_completed_timestamps(std::vector<StreamBucket> &stream_buckets,
                                       Timestamp completed_bound) noexcept;
    void _emit_completed_timestamps(std::vector<StreamBucket> &stream_buckets) noexcept;
    void _handle_event(const PushEvent &event);
    void _handle_event(const CloseEvent &event);
    void _handle_event(const AdvanceEvent &event);

    // thread runner
    void _run();
    void _shutdown();

  public:
    // Constructor
    MedianAggregatorMPSC(std::size_t stream_size);

    MedianAggregatorMPSC(const MedianAggregatorMPSC &medianAggreator) = delete;
    MedianAggregatorMPSC &operator=(const MedianAggregatorMPSC &medianAggreator) = delete;

    // Destructor
    ~MedianAggregatorMPSC();

  public:
    // Public Manipulators
    void push(StreamId stream_id, Record record);
    void close(StreamId stream_id);
    void advance_watermark(StreamId stream_id, Timestamp watermark);
    const std::vector<Value> &emit() const noexcept;
};

} // namespace prep

#endif