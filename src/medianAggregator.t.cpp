#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <medianAggregator.h>
#include <thread>
#include <vector>

using namespace prep;

class MedianAggregatorTest : public ::testing::Test {
  protected:
    void expectMedianValues(const std::vector<double> &expected) {
        const auto &actual = aggregator.emit();
        ASSERT_EQ(actual.size(), expected.size());
        for (size_t i = 0; i < expected.size(); ++i) {
            EXPECT_DOUBLE_EQ(actual[i], expected[i]);
        }
    }

    MedianAggregator aggregator{2};
};

// ============================================================================
// Single Stream Tests
// ============================================================================

TEST_F(MedianAggregatorTest, SingleStreamSingleValue) {
    aggregator.push(0, {1, 10.0});
    aggregator.close(1);
    aggregator.close(0);
    expectMedianValues({10.0});
}

TEST_F(MedianAggregatorTest, SingleStreamMultipleValues) {
    aggregator.push(0, {1, 5.0});
    aggregator.push(0, {2, 15.0});
    aggregator.close(1);
    aggregator.close(0);
    expectMedianValues({5.0, 15.0});
}

// ============================================================================
// Two Streams - Odd Number of Values
// ============================================================================

TEST_F(MedianAggregatorTest, TwoStreamsOddValues) {
    aggregator.push(0, {1, 10.0});
    aggregator.push(1, {1, 20.0});
    aggregator.push(1, {1, 30.0});
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({20.0});
}

TEST_F(MedianAggregatorTest, TwoStreamsMixedTimestamps) {
    aggregator.push(0, {1, 10.0});
    aggregator.push(0, {2, 40.0});
    aggregator.push(1, {1, 20.0});
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({15.0, 40.0});
}

// ============================================================================
// Two Streams - Even Number of Values
// ============================================================================

TEST_F(MedianAggregatorTest, TwoStreamsEvenValuesMedianAverages) {
    aggregator.push(0, {1, 10.0});
    aggregator.push(1, {1, 20.0});
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({15.0});
}

TEST_F(MedianAggregatorTest, TwoStreamsMultipleEvenMedians) {
    aggregator.push(0, {1, 1.0});
    aggregator.push(0, {2, 3.0});
    aggregator.push(1, {1, 5.0});
    aggregator.push(1, {2, 7.0});
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({3.0, 5.0});
}

// ============================================================================
// Timestamp Completion
// ============================================================================

TEST_F(MedianAggregatorTest, TimestampCompletedWhenAllStreamsAdvance) {
    aggregator.push(0, {1, 10.0});
    aggregator.push(1, {2, 20.0}); // Stream 1 advances past timestamp 1
    aggregator.close(0);
    expectMedianValues({10.0}); // Timestamp 1 is now complete
}

TEST_F(MedianAggregatorTest, TimestampCompletedWhenStreamClosed) {
    aggregator.push(0, {1, 10.0});
    aggregator.push(0, {2, 11.0});
    aggregator.close(1);        // Stream 1 closed
    expectMedianValues({10.0}); // Timestamp 1 is now complete
}

TEST_F(MedianAggregatorTest, MultipleTimestampsCompletedInOrder) {
    aggregator.push(0, {1, 1.0});
    aggregator.push(0, {2, 2.0});
    aggregator.push(0, {3, 3.0});
    aggregator.push(1, {5, 50.0}); // Stream 1 jumps ahead
    aggregator.close(0);
    expectMedianValues({1.0, 2.0, 3.0});
}

// ============================================================================
// Watermark Advancement
// ============================================================================

TEST_F(MedianAggregatorTest, AdvanceWatermarkCompletesTimestamps) {
    aggregator.push(0, {1, 10.0});
    aggregator.advance_watermark(1, 2); // Stream 1 watermark advances past timestamp 1
    aggregator.advance_watermark(0, 2); // Stream 0 watermark advances past timestamp 1
    expectMedianValues({10.0});
}

TEST_F(MedianAggregatorTest, AdvanceWatermarkMultipleTimestamps) {
    aggregator.push(0, {1, 10.0});
    aggregator.push(0, {2, 20.0});
    aggregator.push(0, {3, 30.0});
    aggregator.advance_watermark(1, 4);
    aggregator.advance_watermark(0, 4); // Completes all three timestamps
    expectMedianValues({10.0, 20.0, 30.0});
}

TEST_F(MedianAggregatorTest, AdvanceWatermarkWithValues) {
    aggregator.push(0, {1, 10.0});
    aggregator.advance_watermark(1, 2);
    aggregator.push(0, {2, 20.0});
    aggregator.advance_watermark(1, 3);
    aggregator.advance_watermark(0, 3);
    expectMedianValues({10.0, 20.0});
}

TEST_F(MedianAggregatorTest, AdvanceWatermarkNoValuesForTimestamp) {
    aggregator.push(0, {1, 10.0});
    aggregator.push(0, {3, 30.0});
    aggregator.advance_watermark(1, 4);
    aggregator.advance_watermark(0, 4);
    expectMedianValues({10.0, 30.0});
}

TEST_F(MedianAggregatorTest, AdvanceWatermarkThenClose) {
    aggregator.push(0, {1, 10.0});
    aggregator.advance_watermark(1, 2);
    aggregator.advance_watermark(0, 2);
    expectMedianValues({10.0});
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({10.0});
}

TEST_F(MedianAggregatorTest, CloseAfterWatermarkAdvance) {
    aggregator.push(0, {1, 10.0});
    aggregator.push(0, {2, 20.0});
    aggregator.advance_watermark(1, 3);
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({10.0, 20.0});
}

// ============================================================================
// Empty Timestamps
// ============================================================================

TEST_F(MedianAggregatorTest, StreamWithNoRecordForTimestamp) {
    aggregator.push(0, {1, 10.0});
    aggregator.push(0, {3, 30.0});
    aggregator.push(1, {2, 20.0});
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({10.0, 20.0, 30.0});
}

TEST_F(MedianAggregatorTest, CompleteTimestampWithNoValues) {
    aggregator.push(0, {1, 10.0});
    aggregator.push(1, {3, 30.0});
    aggregator.close(0);
    expectMedianValues({10.0});
    aggregator.close(1);
    expectMedianValues({10.0, 30.0});
}

// ============================================================================
// Error Cases
// ============================================================================

TEST_F(MedianAggregatorTest, PushWithLowerTimestampReturnsError) {
    aggregator.push(0, {10, 10.0});
    auto result = aggregator.push(0, {5, 50.0});
    EXPECT_EQ(result, MedianAggregatorError::LOW_TIMESTAMP);
}

TEST_F(MedianAggregatorTest, PushWithSameTimestampSucceeds) {
    aggregator.push(0, {10, 10.0});
    auto result = aggregator.push(0, {10, 20.0});
    EXPECT_EQ(result, MedianAggregatorError::SUCCESS);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(MedianAggregatorTest, AllStreamsClosedWithoutData) {
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({});
}

TEST_F(MedianAggregatorTest, NegativeValues) {
    aggregator.push(0, {1, -10.0});
    aggregator.push(1, {1, -20.0});
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({-15.0});
}

TEST_F(MedianAggregatorTest, MixedPositiveNegative) {
    aggregator.push(0, {1, -5.0});
    aggregator.push(0, {1, 5.0});
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({0.0});
}

TEST_F(MedianAggregatorTest, ZeroValues) {
    aggregator.push(0, {1, 0.0});
    aggregator.push(1, {1, 0.0});
    aggregator.close(0);
    aggregator.close(1);
    expectMedianValues({0.0});
}

// ============================================================================
// Multiple Streams with Complex Patterns
// ============================================================================

TEST_F(MedianAggregatorTest, ThreeStreamMultipleValues) {
    MedianAggregator agg{3};
    agg.push(0, {1, 1.0});
    agg.push(1, {1, 2.0});
    agg.push(2, {1, 3.0});
    agg.close(0);
    agg.close(1);
    agg.close(2);
    const auto &result = agg.emit();
    EXPECT_EQ(result.size(), 1);
    EXPECT_DOUBLE_EQ(result[0], 2.0);
}

TEST_F(MedianAggregatorTest, InterleavedPushesAndCloses) {
    aggregator.push(0, {1, 10.0});
    aggregator.close(0);
    aggregator.push(1, {1, 20.0});
    aggregator.close(1);
    expectMedianValues({15.0});
}

TEST_F(MedianAggregatorTest, LargeGapsBetweenTimestamps) {
    aggregator.push(0, {1, 1.0});
    aggregator.push(1, {1000, 1000.0});
    aggregator.close(0);
    expectMedianValues({1.0});
    aggregator.close(1);
    const auto &result = aggregator.emit();
    EXPECT_EQ(result.size(), 2);
}

// ============================================================================
// Multithreading Tests
// ============================================================================

TEST(MedianAggregatorThreadTest, ConcurrentPushFromMultipleStreams) {
    static constexpr int NUM_STREAMS = 3;
    static constexpr int ITERATIONS = 5;
    MedianAggregator agg{NUM_STREAMS};
    std::vector<std::thread> threads;

    // Each thread pushes values from its assigned stream
    for (size_t stream = 0; stream < NUM_STREAMS; ++stream) {
        threads.emplace_back([&agg, stream]() {
            for (int ts = 1; ts <= ITERATIONS; ++ts) {
                agg.push(stream, {ts, static_cast<double>(stream * 10 + ts)});
            }
        });
    }

    // Close streams
    for (size_t stream = 0; stream < NUM_STREAMS; ++stream) {
        threads.emplace_back([&agg, stream]() { agg.close(stream); });
    }

    for (auto &t : threads) {
        t.join();
    }

    const auto &result = agg.emit();
    EXPECT_EQ(result.size(), ITERATIONS);
}

TEST(MedianAggregatorThreadTest, ConcurrentPushAndEmit) {
    static constexpr int NUM_STREAMS = 2;
    static constexpr int ITERATIONS = 100;
    MedianAggregator agg{NUM_STREAMS};
    std::vector<double> emitted_values;
    std::atomic<bool> done{false};

    std::thread push_thread([&agg]() {
        for (int ts = 1; ts <= ITERATIONS; ++ts) {
            agg.push(0, {ts, static_cast<double>(ts)});
        }
        agg.advance_watermark(1, ITERATIONS + 1);
    });

    std::thread emit_thread([&agg, &emitted_values, &done]() {
        while (!done) {
            const auto &vals = agg.emit();
            emitted_values = vals;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    push_thread.join();
    agg.close(0);
    agg.close(1);
    done = true;
    emit_thread.join();

    const auto &final_result = agg.emit();
    EXPECT_EQ(final_result.size(), ITERATIONS);
}

TEST(MedianAggregatorThreadTest, ConcurrentWatermarkAdvance) {
    static constexpr int NUM_STREAMS = 4;
    static constexpr int ITERATIONS = 10;
    MedianAggregator agg{NUM_STREAMS};

    std::thread push_thread([&agg]() {
        for (int ts = 1; ts <= ITERATIONS; ++ts) {
            agg.push(0, {ts, static_cast<double>(ts * 2)});
        }
    });

    std::vector<std::thread> watermark_threads;
    for (size_t stream = 1; stream < NUM_STREAMS; ++stream) {
        watermark_threads.emplace_back([&agg, stream]() { agg.advance_watermark(stream, 11); });
    }

    push_thread.join();
    for (auto &t : watermark_threads) {
        t.join();
    }

    agg.close(0);
    agg.close(1);
    agg.close(2);
    agg.close(3);

    const auto &result = agg.emit();
    EXPECT_EQ(result.size(), ITERATIONS);
}

TEST(MedianAggregatorThreadTest, StressTestHighConcurrency) {
    static constexpr int ITERATIONS = 50;
    static constexpr int NUM_STREAMS = 5;
    MedianAggregator agg{NUM_STREAMS};
    std::vector<std::thread> threads;

    // Push threads for each stream
    for (size_t stream = 0; stream < NUM_STREAMS; ++stream) {
        threads.emplace_back([&agg, stream]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                agg.push(stream, {i + 1, static_cast<double>(stream * 100 + i)});
                agg.advance_watermark(stream, i + 1);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    for (size_t stream = 0; stream < NUM_STREAMS; ++stream) {
        agg.close(stream);
    }

    const auto &result = agg.emit();
    EXPECT_EQ(result.size(), ITERATIONS);
}

TEST(MedianAggregatorThreadTest, MultiplePushesToSameTimestamp) {
    static constexpr int NUM_STREAMS = 2;
    MedianAggregator agg{NUM_STREAMS};
    static constexpr int NUM_VALUES_PER_STREAM = 100;

    std::vector<std::thread> threads;
    for (size_t stream = 0; stream < NUM_STREAMS; ++stream) {
        threads.emplace_back([&agg, stream]() {
            for (int i = 0; i < NUM_VALUES_PER_STREAM; ++i) {
                agg.push(stream, {1, static_cast<double>(stream * 1000 + i)});
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    agg.close(0);
    agg.close(1);

    const auto &result = agg.emit();
    EXPECT_EQ(result.size(), 1);
    EXPECT_GE(result[0], 0.0);
}

TEST(MedianAggregatorThreadTest, RaceBetweenCloseAndPush) {
    MedianAggregator agg{2};

    std::thread push_thread([&agg]() {
        for (int ts = 1; ts <= 20; ++ts) {
            agg.push(0, {ts, static_cast<double>(ts)});
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });

    std::thread close_thread([&agg]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        agg.close(0);
        agg.close(1);
    });

    push_thread.join();
    close_thread.join();

    const auto &result = agg.emit();
    EXPECT_GE(result.size(), 0);
    EXPECT_LE(result.size(), 20);
}
