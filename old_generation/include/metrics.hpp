#ifndef METRICS_HPP
#define METRICS_HPP

#include <cstdint>
#include <cstddef>
#include <chrono>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <cstring>

#ifdef __linux__
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace bench {

// Estrutura de Contadores Algoritmicos (Passagem 2)
struct AlgorithmMetrics {
    uint64_t comparisons = 0;
    uint64_t swaps = 0;
    uint64_t writes = 0;
    size_t current_aux_bytes = 0;
    size_t peak_aux_bytes = 0;
    size_t current_recursion_depth = 0;
    size_t max_recursion_depth = 0;

    static inline AlgorithmMetrics& instance() noexcept {
        static AlgorithmMetrics inst;
        return inst;
    }

    inline void reset() noexcept {
        comparisons = 0;
        swaps = 0;
        writes = 0;
        current_aux_bytes = 0;
        peak_aux_bytes = 0;
        current_recursion_depth = 0;
        max_recursion_depth = 0;
    }

    inline void add_aux_memory(size_t bytes) noexcept {
        current_aux_bytes += bytes;
        if (current_aux_bytes > peak_aux_bytes) {
            peak_aux_bytes = current_aux_bytes;
        }
    }

    inline void remove_aux_memory(size_t bytes) noexcept {
        if (current_aux_bytes >= bytes) {
            current_aux_bytes -= bytes;
        } else {
            current_aux_bytes = 0;
        }
    }

    inline void enter_recursion() noexcept {
        ++current_recursion_depth;
        if (current_recursion_depth > max_recursion_depth) {
            max_recursion_depth = current_recursion_depth;
        }
    }

    inline void exit_recursion() noexcept {
        if (current_recursion_depth > 0) {
            --current_recursion_depth;
        }
    }
};

// Wrapper TrackedInt32 para Instrumentacao Exata na Passagem 2
struct TrackedInt32 {
    int32_t value;

    TrackedInt32() noexcept : value(0) {}
    TrackedInt32(int32_t v) noexcept : value(v) {}

    TrackedInt32(const TrackedInt32& other) noexcept : value(other.value) {
        AlgorithmMetrics::instance().writes++;
    }

    TrackedInt32(TrackedInt32&& other) noexcept : value(other.value) {
        AlgorithmMetrics::instance().writes++;
    }

    TrackedInt32& operator=(const TrackedInt32& other) noexcept {
        value = other.value;
        AlgorithmMetrics::instance().writes++;
        return *this;
    }

    TrackedInt32& operator=(TrackedInt32&& other) noexcept {
        value = other.value;
        AlgorithmMetrics::instance().writes++;
        return *this;
    }

    TrackedInt32& operator=(int32_t v) noexcept {
        value = v;
        AlgorithmMetrics::instance().writes++;
        return *this;
    }

    inline bool operator<(const TrackedInt32& rhs) const noexcept {
        AlgorithmMetrics::instance().comparisons++;
        return value < rhs.value;
    }

    inline bool operator>(const TrackedInt32& rhs) const noexcept {
        AlgorithmMetrics::instance().comparisons++;
        return value > rhs.value;
    }

    inline bool operator<=(const TrackedInt32& rhs) const noexcept {
        AlgorithmMetrics::instance().comparisons++;
        return value <= rhs.value;
    }

    inline bool operator>=(const TrackedInt32& rhs) const noexcept {
        AlgorithmMetrics::instance().comparisons++;
        return value >= rhs.value;
    }

    inline bool operator==(const TrackedInt32& rhs) const noexcept {
        AlgorithmMetrics::instance().comparisons++;
        return value == rhs.value;
    }

    inline bool operator!=(const TrackedInt32& rhs) const noexcept {
        AlgorithmMetrics::instance().comparisons++;
        return value != rhs.value;
    }
};

template <typename T>
struct IsTracked : std::false_type {};

template <>
struct IsTracked<TrackedInt32> : std::true_type {};

template <typename T>
inline void alg_swap(T& a, T& b) noexcept {
    if constexpr (IsTracked<T>::value) {
        AlgorithmMetrics::instance().swaps++;
        AlgorithmMetrics::instance().writes += 3;
        std::swap(a.value, b.value);
    } else {
        std::swap(a, b);
    }
}

template <typename T>
struct RecursionGuard {
    inline RecursionGuard() noexcept {
        if constexpr (IsTracked<T>::value) {
            AlgorithmMetrics::instance().enter_recursion();
        }
    }
    inline ~RecursionGuard() noexcept {
        if constexpr (IsTracked<T>::value) {
            AlgorithmMetrics::instance().exit_recursion();
        }
    }
};

template <typename T>
struct AuxMemoryGuard {
    size_t allocated_bytes;
    inline explicit AuxMemoryGuard(size_t bytes) noexcept : allocated_bytes(bytes) {
        if constexpr (IsTracked<T>::value) {
            AlgorithmMetrics::instance().add_aux_memory(allocated_bytes);
        }
    }
    inline ~AuxMemoryGuard() noexcept {
        if constexpr (IsTracked<T>::value) {
            AlgorithmMetrics::instance().remove_aux_memory(allocated_bytes);
        }
    }
};

// Contadores de Hardware via perf_event_open (com fallback automatico em Windows/WSL)
struct HardwareMetrics {
    uint64_t cpu_cycles = 0;
    uint64_t instructions = 0;
    uint64_t branch_misses = 0;
    uint64_t cache_misses = 0;
    double ipc = 0.0;
    bool available = false;
};

class PerfCounters {
private:
    int fds_[4];
    bool available_;

#ifdef __linux__
    static long perf_event_open_syscall(struct perf_event_attr* hw_event, pid_t pid,
                                        int cpu, int group_fd, unsigned long flags) {
        return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
    }

    int open_counter(uint32_t type, uint64_t config) {
        struct perf_event_attr pe;
        std::memset(&pe, 0, sizeof(struct perf_event_attr));
        pe.type = type;
        pe.size = sizeof(struct perf_event_attr);
        pe.config = config;
        pe.disabled = 1;
        pe.exclude_kernel = 1;
        pe.exclude_hv = 1;
        return static_cast<int>(perf_event_open_syscall(&pe, 0, -1, -1, 0));
    }
#endif

public:
    PerfCounters() : fds_{-1, -1, -1, -1}, available_(false) {
#ifdef __linux__
        fds_[0] = open_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
        fds_[1] = open_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
        fds_[2] = open_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
        fds_[3] = open_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
        if (fds_[0] >= 0 && fds_[1] >= 0) {
            available_ = true;
        }
#endif
    }

    ~PerfCounters() {
#ifdef __linux__
        for (int i = 0; i < 4; ++i) {
            if (fds_[i] >= 0) {
                close(fds_[i]);
            }
        }
#endif
    }

    inline bool is_available() const noexcept { return available_; }

    inline void start() noexcept {
#ifdef __linux__
        if (!available_) return;
        for (int i = 0; i < 4; ++i) {
            if (fds_[i] >= 0) {
                ioctl(fds_[i], PERF_EVENT_IOC_RESET, 0);
                ioctl(fds_[i], PERF_EVENT_IOC_ENABLE, 0);
            }
        }
#endif
    }

    inline HardwareMetrics stop() noexcept {
        HardwareMetrics m;
        m.available = available_;
#ifdef __linux__
        if (!available_) return m;
        uint64_t vals[4] = {0, 0, 0, 0};
        for (int i = 0; i < 4; ++i) {
            if (fds_[i] >= 0) {
                ioctl(fds_[i], PERF_EVENT_IOC_DISABLE, 0);
                ssize_t res = read(fds_[i], &vals[i], sizeof(uint64_t));
                (void)res;
            }
        }
        m.cpu_cycles = vals[0];
        m.instructions = vals[1];
        m.branch_misses = vals[2];
        m.cache_misses = vals[3];
        m.ipc = (m.cpu_cycles > 0)
                    ? static_cast<double>(m.instructions) / static_cast<double>(m.cpu_cycles)
                    : 0.0;
#endif
        return m;
    }
};

} // namespace bench

#endif // METRICS_HPP
