#ifndef PERFUTIL
#define PERFUTIL

#include <cstdint>

namespace perfutil {
int compare(char c1, char c2);

bool compare1(const char *s1, const char *s2, uint32_t l);

bool compare2(const char *s1, const char *s2, uint32_t l);

void computeSortPerformance(const uint32_t l, const uint32_t n);

void compareHardwareTimer(const double y);
} // namespace perfutil

#endif