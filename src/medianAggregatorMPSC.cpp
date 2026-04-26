#include <algorithm>
#include <limits>
#include <medianAggregatorMPSC.h>
#include <mutex>
#include <numeric>
#include <stdexcept>
#include <variant>

namespace prep {
MedianAggregatorMPSC::MedianAggregatorMPSC(std::size_t stream_size)
    : d_latest_stream_timestamps(stream_size, std::numeric_limits<Timestamp>::min()),
      d_consumer(&MedianAggregatorMPSC::_run, this) 
    , d_stopping(false){
    d_emitted_records.reserve(1 << 10);
}

MedianAggregatorMPSC::~MedianAggregatorMPSC() { _shutdown(); }

double MedianAggregatorMPSC::_compute_median_value(std::vector<Value> &values) noexcept {
    if (values.empty()) {
        return 0;
    }

    size_t mid = values.size() / 2;
    std::nth_element(values.begin(), values.begin() + mid, values.end());
    if (values.size() % 2 != 0) {
        return values[mid];
    } else {
        auto upper_bound = values[mid];
        auto lower_bound = *std::max_element(values.begin(), values.begin() + mid);
        return std::midpoint(upper_bound, lower_bound);
    }
}

void MedianAggregatorMPSC::_collect_completed_timestamps(std::vector<StreamBucket> &stream_buckets,
                                                         Timestamp completed_bound) noexcept {
    auto iter = d_pending_by_timestamp.begin();
    while (!d_pending_by_timestamp.empty() && iter->first < completed_bound) {
        stream_buckets.emplace_back(iter->second);
        auto tmp = iter;
        ++iter;
        d_pending_by_timestamp.erase(tmp);
    }
}

void MedianAggregatorMPSC::_emit_completed_timestamps(
    std::vector<StreamBucket> &stream_buckets) noexcept {
    for (auto &bucket : stream_buckets) {
        auto mid_value = _compute_median_value(bucket);
        d_emitted_records.emplace_back(mid_value);
    }
}

void MedianAggregatorMPSC::_handle_event(const PushEvent &event) {
    std::vector<StreamBucket> stream_buckets;
    if (d_latest_stream_timestamps[event.stream_id] > event.record.timestamp) {
        throw std::runtime_error("Low Timestamp");
    }
    d_pending_by_timestamp[event.record.timestamp].emplace_back(event.record.value);
    d_latest_stream_timestamps[event.stream_id] = event.record.timestamp;
    const auto completed_bound = std::ranges::min(d_latest_stream_timestamps);
    _collect_completed_timestamps(stream_buckets, completed_bound);
    _emit_completed_timestamps(stream_buckets);
}

void MedianAggregatorMPSC::_handle_event(const CloseEvent &event) {
    std::vector<StreamBucket> stream_buckets;
    d_latest_stream_timestamps[event.stream_id] = std::numeric_limits<Timestamp>::max();
    const auto completed_bound = std::ranges::min(d_latest_stream_timestamps);
    _collect_completed_timestamps(stream_buckets, completed_bound);
    _emit_completed_timestamps(stream_buckets);
}

void MedianAggregatorMPSC::_handle_event(const AdvanceEvent &event) {
    std::vector<StreamBucket> stream_buckets;
    d_latest_stream_timestamps[event.stream_id] = event.watermark;
    const auto completed_bound = std::ranges::min(d_latest_stream_timestamps);
    _collect_completed_timestamps(stream_buckets, completed_bound);
    _emit_completed_timestamps(stream_buckets);
}

void MedianAggregatorMPSC::_run() {
    for (;;) {
        Event event;
        {
            std::unique_lock<std::mutex> unique_lock(d_queue_mutex);
            d_queue_cv.wait(unique_lock, [&] { return d_stopping || !d_queue.empty(); });

            if (d_stopping && d_queue.empty()) {
                break;
            }

            event = std::move(d_queue.front());
            d_queue.pop_front();
        }

        std::visit([&](auto &&e) { _handle_event(e); }, event);
    }
}

void MedianAggregatorMPSC::_shutdown() {
    {
        std::lock_guard<std::mutex> lock_guard{d_queue_mutex};
        if (d_stopping) {
            return;
        }
        d_stopping = true;
    }
    d_queue_cv.notify_one();
    if (d_consumer.joinable()) {
        d_consumer.join();
    }
}

void MedianAggregatorMPSC::push(StreamId stream_id, Record record) {
    {
        std::lock_guard<std::mutex> lock_guard{d_queue_mutex};
        if (d_stopping) {
            throw std::runtime_error{"Aggregator is stopping!"};
        }
        d_queue.emplace_back(PushEvent{stream_id, record});
    }
    d_queue_cv.notify_one();
}

void MedianAggregatorMPSC::close(StreamId stream_id) {
    {
        std::lock_guard<std::mutex> lock_guard{d_queue_mutex};
        if (d_stopping) {
            throw std::runtime_error{"Aggregator is stopping!"};
        }
        d_queue.emplace_back(CloseEvent{stream_id});
    }
    d_queue_cv.notify_one();
}
void MedianAggregatorMPSC::advance_watermark(StreamId stream_id, Timestamp watermark) {
    {
        std::lock_guard<std::mutex> lock_guard{d_queue_mutex};
        if (d_stopping) {
            throw std::runtime_error{"Aggregator is stopping!"};
        }
        d_queue.emplace_back(AdvanceEvent{stream_id, watermark});
    }
    d_queue_cv.notify_one();
}

const std::vector<MedianAggregatorMPSC::Value> &MedianAggregatorMPSC::emit() const noexcept {
    return d_emitted_records;
}
} // namespace prep