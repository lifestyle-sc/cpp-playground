#include <algorithm>
#include <cstddef>
#include <limits>
#include <medianAggregator.h>
#include <mutex>
#include <numeric>

namespace prep {
MedianAggregator::MedianAggregator(std::size_t stream_size)
    : d_latest_stream_timestamp(stream_size, std::numeric_limits<Timestamp>::min()) {
    d_emitted_records.reserve(1 << 10);
}

double MedianAggregator::_compute_median_value(std::vector<Value> &values) noexcept {
    if (values.empty())
        return 0;

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

void MedianAggregator::_collect_completed_timestamps(std::vector<StreamBucket> &stream_buckets,
                                                     Timestamp completed_bound) noexcept {
    auto iter = d_pending_by_timestamp.begin();
    while (!d_pending_by_timestamp.empty() && iter->first < completed_bound) {
        stream_buckets.emplace_back(iter->second);
        auto tmp = iter;
        ++iter;
        d_pending_by_timestamp.erase(tmp);
    }
}

void MedianAggregator::_emit_completed_timestamps(
    std::vector<StreamBucket> &stream_buckets) noexcept {
    for (auto &bucket : stream_buckets) {
        auto mid_value = _compute_median_value(bucket);
        d_emitted_records.emplace_back(mid_value);
    }
}

MedianAggregatorError MedianAggregator::push(StreamId stream_id, Record record) {
    std::vector<StreamBucket> stream_buckets;
    {
        std::lock_guard<std::mutex> lock_guard{d_mutex};
        if (d_latest_stream_timestamp[stream_id] > record.timestamp) {
            return MedianAggregatorError::LOW_TIMESTAMP;
        }
        d_pending_by_timestamp[record.timestamp].emplace_back(record.value);
        d_latest_stream_timestamp[stream_id] = record.timestamp;

        const auto completed_bound = std::ranges::min(d_latest_stream_timestamp);
        _collect_completed_timestamps(stream_buckets, completed_bound);
    }

    _emit_completed_timestamps(stream_buckets);

    return MedianAggregatorError::SUCCESS;
}

void MedianAggregator::close(StreamId stream_id) {
    std::vector<StreamBucket> stream_buckets;
    {
        std::lock_guard<std::mutex> lock_guard{d_mutex};
        d_latest_stream_timestamp[stream_id] = std::numeric_limits<Timestamp>::max();
        const auto completed_bound = std::ranges::min(d_latest_stream_timestamp);
        _collect_completed_timestamps(stream_buckets, completed_bound);
    }
    _emit_completed_timestamps(stream_buckets);
}

void MedianAggregator::advance_watermark(StreamId stream_id, Timestamp watermark) noexcept {
    std::vector<StreamBucket> stream_buckets;
    {
        std::lock_guard<std::mutex> lock_guard{d_mutex};
        d_latest_stream_timestamp[stream_id] = watermark;
        const auto completed_bound = std::ranges::min(d_latest_stream_timestamp);
        _collect_completed_timestamps(stream_buckets, completed_bound);
    }
    _emit_completed_timestamps(stream_buckets);
}

const std::vector<MedianAggregator::Value> &MedianAggregator::emit() const noexcept {
    return d_emitted_records;
}

} // namespace prep