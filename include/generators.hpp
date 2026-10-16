#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include <vector>
#include <cstdint>
#include <random>
#include <algorithm>
#include <string>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace bench {

enum class Distribution {
    RANDOM_UNIFORM,
    SORTED,
    REVERSE_SORTED,
    NEARLY_SORTED,
    SORTED_RUNS,
    ORGAN_PIPE
};

inline std::string distribution_to_string(Distribution d) {
    switch (d) {
        case Distribution::RANDOM_UNIFORM: return "random_uniform";
        case Distribution::SORTED:         return "sorted";
        case Distribution::REVERSE_SORTED: return "reverse_sorted";
        case Distribution::NEARLY_SORTED:  return "nearly_sorted";
        case Distribution::SORTED_RUNS:    return "sorted_runs";
        case Distribution::ORGAN_PIPE:     return "organ_pipe";
    }
    return "unknown";
}

constexpr uint32_t BASE_SEED = 20260818U;

inline std::vector<int32_t> generate_dataset(size_t n, Distribution dist, uint32_t seed = BASE_SEED) {
    std::vector<int32_t> data(n);
    if (n == 0) return data;

    std::mt19937 rng(seed);

    switch (dist) {
        case Distribution::RANDOM_UNIFORM: {
            std::uniform_int_distribution<int32_t> uni(
                std::numeric_limits<int32_t>::min() / 2,
                std::numeric_limits<int32_t>::max() / 2
            );
            for (size_t i = 0; i < n; ++i) {
                data[i] = uni(rng);
            }
            break;
        }

        case Distribution::SORTED: {
            for (size_t i = 0; i < n; ++i) {
                data[i] = static_cast<int32_t>(i);
            }
            break;
        }

        case Distribution::REVERSE_SORTED: {
            for (size_t i = 0; i < n; ++i) {
                data[i] = static_cast<int32_t>(n - 1 - i);
            }
            break;
        }

        case Distribution::NEARLY_SORTED: {
            for (size_t i = 0; i < n; ++i) {
                data[i] = static_cast<int32_t>(i);
            }
            size_t num_swaps = std::max<size_t>(1, n / 200);
            std::uniform_int_distribution<size_t> idx_dist(0, n - 1);
            for (size_t k = 0; k < num_swaps; ++k) {
                size_t i = idx_dist(rng);
                size_t j = idx_dist(rng);
                std::swap(data[i], data[j]);
            }
            break;
        }

        case Distribution::SORTED_RUNS: {
            std::uniform_int_distribution<int32_t> val_dist(-1000000, 1000000);
            for (size_t i = 0; i < n; ++i) {
                data[i] = val_dist(rng);
            }
            size_t min_run = std::max<size_t>(8, static_cast<size_t>(std::sqrt(static_cast<double>(n)) / 2.0));
            size_t max_run = std::max<size_t>(min_run + 8, static_cast<size_t>(std::sqrt(static_cast<double>(n)) * 2.0));
            std::uniform_int_distribution<size_t> run_dist(min_run, max_run);

            size_t pos = 0;
            while (pos < n) {
                size_t len = std::min<size_t>(n - pos, run_dist(rng));
                std::sort(data.begin() + pos, data.begin() + pos + len);
                pos += len;
            }
            break;
        }

        case Distribution::ORGAN_PIPE: {
            size_t mid = n / 2;
            for (size_t i = 0; i < mid; ++i) {
                data[i] = static_cast<int32_t>(i);
            }
            for (size_t i = mid; i < n; ++i) {
                data[i] = static_cast<int32_t>(n - 1 - i);
            }
            break;
        }
    }

    return data;
}

} // namespace bench

#endif // GENERATORS_HPP
