#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <perfUtil.h>

#include <iostream>

bool compareS1(const char *s1, const char *s2) {
    int i1 = 0, i2 = 0;
    char c1, c2;

    while (1) {
        c1 = s1[i1];
        c2 = s2[i2];
        if (c1 != c2) {
            return c1 > c2;
        }

        ++i1;
        ++i2;
    }
}
bool compareS2(const char *s1, const char *s2) {
    uint32_t i1 = 0, i2 = 0;
    char c1, c2;

    while (1) {
        c1 = s1[i1];
        c2 = s2[i2];
        if (c1 != c2) {
            return c1 > c2;
        }

        ++i1;
        ++i2;
    }
}

int main(int argc, char *argv[]) {
    std::cout << "Welcome to my setup" << std::endl;

    // perfutil::computeSortPerformance(1 << 18, 1 << 14);

    // perfutil::compareHardwareTimer(1e6);

    constexpr uint32_t N = 1 << 20;
    constexpr int NI = 1 << 11;
    std::unique_ptr<char[]> s(new char[2 * N]);
    ::memset(s.get(), 'a', 2 * N * sizeof(char));
    s[2 * N - 1] = 0;
    bool sink;
    auto t0 = std::chrono::system_clock::now();
    for (int i = 0; i < NI; ++i) {
        sink = compareS1(s.get(), s.get() + N);
    }
    auto t1 = std::chrono::system_clock::now();
    for (int i = 0; i < NI; ++i) {
        sink = compareS2(s.get(), s.get() + N);
    }
    auto t2 = std::chrono::system_clock::now();

    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() << "us "
              << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << "us"
              << sink << std::endl;
}
