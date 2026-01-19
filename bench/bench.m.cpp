#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>

#define MCA_START __asm volatile("# LLVM-MCA-BEGIN");
#define MCA_END __asm volatile("# LLVM-MCA-END");

bool compareInt(const char *s1, const char *s2) {
    char c1, c2;
    for (int i1 = 0, i2 = 0;; ++i1, ++i2) {
        c1 = s1[i1];
        c2 = s2[i2];
        if (c1 != c2) {
            return c1 > c2;
        }
    }
}

void BM_loop_int(benchmark::State &state) {
    const uint32_t N = state.range(0);
    std::unique_ptr<char[]> s(new char[2 * N]);
    ::memset(s.get(), 'a', 2 * N * sizeof(char));
    s[2 * N - 1] = 0;
    const char *s1 = s.get();
    const char *s2 = s.get() + N;
    for (auto _ : state) {
        benchmark::DoNotOptimize(compareInt(s1, s2));
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_add(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    for (auto _ : state) {
        unsigned long a1 = 0;
        for (size_t i = 0; i < N; ++i) {
            a1 += p1[i] + p2[i];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_add_optimised_conditional(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    for (auto _ : state) {
        unsigned long a1 = 0;
        for (size_t i = 0; i < N; ++i) {
            a1 += p1[i] > p2[i] ? p1[i] : p2[i];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}
void BM_add_conditional(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 1;
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    int *b1 = c1.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            MCA_START
            if (b1[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
            MCA_END
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}
void BM_add_conditional_predicted(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() >= 0;
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    int *b1 = c1.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            MCA_START
            if (b1[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
            MCA_END
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_substract(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    for (auto _ : state) {
        unsigned long a1 = 0;
        for (size_t i = 0; i < N; ++i) {
            a1 += p1[i] - p2[i];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}
void BM_multiply(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    for (auto _ : state) {
        unsigned long a1 = 0;
        for (size_t i = 0; i < N; ++i) {
            a1 += p1[i] * p2[i];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}
void BM_divide(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    for (auto _ : state) {
        unsigned long a1 = 0;
        for (size_t i = 0; i < N; ++i) {
            a1 += p1[i] / p2[i];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}
void BM_add_multiply(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            a1 += p1[i] + p2[i];
            a2 += p1[i] * p2[i];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}
void BM_add_multiply_sub_shift(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0, a3 = 0, a4 = 0;
        for (size_t i = 0; i < N; ++i) {
            a1 += p1[i] + p2[i];
            a2 += p1[i] * p2[i];
            a3 += p1[i] - p2[i];
            a4 += p1[i] << 2;
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::DoNotOptimize(a3);
        benchmark::DoNotOptimize(a4);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_instructions(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
        for (size_t i = 0; i < N; ++i) {
            a1 += p1[i] + p2[i];
            a2 += p1[i] * p2[i];
            a3 += p2[i] - p1[i];
            a4 += p1[i] << 2;
            a5 += (p2[i] << 1) * p2[i];
            a6 += (p2[i] - 3) * p1[i];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::DoNotOptimize(a3);
        benchmark::DoNotOptimize(a4);
        benchmark::DoNotOptimize(a5);
        benchmark::DoNotOptimize(a6);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_add_sub_multiply(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    for (auto _ : state) {
        unsigned long a1 = 0;
        for (size_t i = 0; i < N; ++i) {
            a1 += (p1[i] + p2[i]) * (p1[i] - p2[i]);
        }
        benchmark::DoNotOptimize(a1);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_branch_unoptimized(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    int *b1 = c1.data();
    int *b2 = c2.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            MCA_START
            if (b1[i] || b2[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
            MCA_END
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_branch_temp(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    int *b1 = c1.data();
    int *b2 = c2.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            MCA_START
            bool isValid = b1[i] || b2[i];
            if (isValid) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
            MCA_END
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_branch_vtemp(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    std::vector<bool> c3(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
        c3[i] = c1[i] || c2[i];
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            MCA_START
            if (c3[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
            MCA_END
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_branch_add(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    int *b1 = c1.data();
    int *b2 = c2.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            MCA_START
            if (b1[i] + b2[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
            MCA_END
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_branch_bitwiseOR(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    int *b1 = c1.data();
    int *b2 = c2.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            MCA_START
            if (b1[i] | b2[i]) {
                a1 += p1[i];
            } else {
                a1 *= p2[i];
            }
            MCA_END
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_addBranchlessEven(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
    }

    unsigned long *p1 = v1.data();
    unsigned long *p2 = v2.data();
    for (auto _ : state) {
        unsigned long a1 = 0;
        // Given N is an even number
        for (size_t i = 0; i < N; i += 2) {
            a1 += p1[i] + p2[i] + p1[i + 1] + p2[i + 1];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_conditionalBranch(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    std::vector<bool> c3(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
        c3[i] = c1[i] || c2[i];
    }

    unsigned long *p1 = v1.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            MCA_START
            if (c3[i]) {
                a1 += p1[i];
            } else {
                a2 *= p1[i];
            }
            MCA_END
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_lookUpArrayBranchless(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    std::vector<bool> c3(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
        c3[i] = c1[i] || c2[i];
    }

    unsigned long *p1 = v1.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        unsigned long *a[2] = {&a2, &a1};
        for (size_t i = 0; i < N; ++i) {
            a[c3[i]] += p1[i];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

void BM_conditionalOperatorBranchless(benchmark::State &state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N);
    std::vector<int> c1(N);
    std::vector<int> c2(N);
    std::vector<bool> c3(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        c1[i] = rand() & 0x1;
        c2[i] = !c1[i];
        c3[i] = c1[i] || c2[i];
    }

    unsigned long *p1 = v1.data();

    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            (c3[i] ? a1 : a2) += p1[i];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N * state.iterations());
}

// BENCHMARK(BM_loop_int)->Arg(1 << 20);
// BENCHMARK(BM_add)->Arg(1 << 22);
// BENCHMARK(BM_substract)->Arg(1 << 22);
// BENCHMARK(BM_multiply)->Arg(1 << 22);
// BENCHMARK(BM_add_multiply)->Arg(1 << 22);
// BENCHMARK(BM_add_multiply_sub_shift)->Arg(1 << 22);
// BENCHMARK(BM_instructions)->Arg(1 << 22);
// BENCHMARK(BM_add_sub_multiply)->Arg(1 << 22);
// BENCHMARK(BM_add_optimised_conditional)->Arg(1 << 22);
// BENCHMARK(BM_add_conditional)->Arg(1 << 22);
// BENCHMARK(BM_add_conditional_predicted)->Arg(1 << 22);
// BENCHMARK(BM_branch_unoptimized)->Arg(1 << 22);
// BENCHMARK(BM_branch_temp)->Arg(1 << 22);
// BENCHMARK(BM_branch_vtemp)->Arg(1 << 22);
// BENCHMARK(BM_branch_add)->Arg(1 << 22);
// BENCHMARK(BM_branch_bitwiseOR)->Arg(1 << 22);
// BENCHMARK(BM_addBranchlessEven)->Arg(1 << 22);
BENCHMARK(BM_conditionalBranch)->Arg(1 << 22);
BENCHMARK(BM_lookUpArrayBranchless)->Arg(1 << 22);
BENCHMARK(BM_conditionalOperatorBranchless)->Arg(1 << 22);
BENCHMARK_MAIN();