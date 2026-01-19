#include <perfUtil.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <future>
#include <iostream>
#include <memory>
#include <random>
#include <unistd.h>
#include <vector>

namespace perfutil {
namespace {
double duration(timespec a, timespec b) {
    return a.tv_sec - b.tv_sec + 1e-9 * (a.tv_nsec - b.tv_nsec);
}
} // namespace

bool compare1(const char *s1, const char *s2, uint32_t l) {
    if (s1 == s2) {
        return false;
    }

    for (int i1 = 0, i2 = 0;; ++i1, ++i2) {
        int res = compare(s1[i1], s2[i2]);
        if (res != 0)
            return res > 0;
    }
    return false;
}

bool compare2(const char *s1, const char *s2, uint32_t l) {
    if (s1 == s2) {
        return false;
    }

    for (int i1 = 0, i2 = 0;; ++i1, ++i2) {
        int res = compare(s1[i1], s2[i2]);
        if (res != 0)
            return res < 0;
    }
    return false;
}

int compare(char c1, char c2) {
    if (c1 > c2) {
        return 1;
    }

    if (c1 < c2) {
        return -1;
    }

    return 0;
}

void computeSortPerformance(const uint32_t l, const uint32_t n) {
    auto s = std::make_unique<char[]>(l);
    std::vector<const char *> vs(n);

    // prepare the string
    std::minstd_rand rgen;
    ::memset(s.get(), 'a', n * sizeof(char));
    for (uint32_t i = 0; i < l / 1024; ++i) {
        s[rgen() % (l - 1)] = 'a' + (rgen() % ('z' - 'a' + 1));
    }

    s[l - 1] = 0;
    for (uint32_t i = 0; i < n; ++i) {
        vs[i] = &s[rgen() % (l - 1)];
    }

    size_t count = 0;
    auto t1 = std::chrono::system_clock::now();
    std::sort(vs.begin(), vs.end(), [&count, &l](const char *a, const char *b) {
        ++count;
        return compare1(a, b, l);
    });

    std::sort(vs.begin(), vs.end(), [&count, &l](const char *a, const char *b) {
        ++count;
        return compare2(a, b, l);
    });
    auto t2 = std::chrono::system_clock::now();

    std::cout << "ComputeSortPerformance: Sort time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms ("
              << count << " comparisons)" << std::endl;
}

void compareHardwareTimer(const double y) {
    timespec rt0, ct0, tt0;
    timespec rt1, ct1, tt1;
    double s1 = 0;
    double s2 = 0;
    (void)s2;
    clock_gettime(CLOCK_REALTIME, &rt0);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct0);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt0);
    auto f = std::async(std::launch::async, [&y, &s1]() {
        for (double x = 0; x < y; x += 0.1) {
            s1 += sin(x);
        }
    });
    for (double x = 0; x < y; x += 0.1) {
        s2 += sin(x);
    }
    f.wait();
    // sleep(1);
    clock_gettime(CLOCK_REALTIME, &rt1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ct1);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tt1);

    std::cout << "CompareHardwareTimer: Real time: " << duration(rt1, rt0)
              << "s, CPU time: " << duration(ct1, ct0) << "s, Thread time: " << duration(tt1, tt0)
              << "s" << std::endl;
}
} // namespace perfutil