#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include "metrics.hpp"
#include <vector>
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <utility>

namespace bench {

// 1. BUBBLE SORT
template <typename T>
void bubble_sort(std::vector<T>& arr) {
    RecursionGuard<T> rec_guard;
    AuxMemoryGuard<T> mem_guard(0);

    const size_t n = arr.size();
    if (n < 2) return;

    for (size_t i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (size_t j = 0; j < n - 1 - i; ++j) {
            if (arr[j + 1] < arr[j]) {
                alg_swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

// 2. INSERTION SORT
template <typename T>
inline void insertion_sort_range(T* begin, T* end) {
    if (end - begin < 2) return;

    for (T* it = begin + 1; it < end; ++it) {
        if (*it < *(it - 1)) {
            T key = std::move(*it);
            T* j = it;
            do {
                *j = std::move(*(j - 1));
                --j;
            } while (j > begin && key < *(j - 1));
            *j = std::move(key);
        }
    }
}

template <typename T>
void insertion_sort(std::vector<T>& arr) {
    RecursionGuard<T> rec_guard;
    AuxMemoryGuard<T> mem_guard(0);
    if (arr.size() < 2) return;
    insertion_sort_range(arr.data(), arr.data() + arr.size());
}

// 3. MERGE SORT
namespace detail {

template <typename T>
void mergesort_rec(T* arr, T* buf, size_t left, size_t right) {
    RecursionGuard<T> rec_guard;
    if (right - left <= 1) return;

    size_t mid = left + (right - left) / 2;
    mergesort_rec(arr, buf, left, mid);
    mergesort_rec(arr, buf, mid, right);

    if (!(arr[mid] < arr[mid - 1])) return;

    size_t i = left;
    size_t j = mid;
    size_t k = left;

    while (i < mid && j < right) {
        if (!(arr[j] < arr[i])) {
            buf[k++] = std::move(arr[i++]);
        } else {
            buf[k++] = std::move(arr[j++]);
        }
    }
    while (i < mid) {
        buf[k++] = std::move(arr[i++]);
    }
    while (j < right) {
        buf[k++] = std::move(arr[j++]);
    }
    for (size_t idx = left; idx < right; ++idx) {
        arr[idx] = std::move(buf[idx]);
    }
}

} // namespace detail

template <typename T>
void merge_sort(std::vector<T>& arr) {
    const size_t n = arr.size();
    if (n < 2) return;

    AuxMemoryGuard<T> mem_guard(n * sizeof(int32_t));
    std::vector<T> buf(n);
    detail::mergesort_rec(arr.data(), buf.data(), 0, n);
}

// 4. POWERSORT (Munro & Wild)
namespace powersort_detail {

constexpr size_t MIN_RUN_LEN = 24;

template <typename T>
size_t extend_and_normalize_run(T* arr, size_t start, size_t n) {
    if (start + 1 >= n) return n;

    size_t end = start + 2;
    if (arr[start + 1] < arr[start]) {
        while (end < n && arr[end] < arr[end - 1]) {
            ++end;
        }
        size_t l = start, r = end - 1;
        while (l < r) {
            alg_swap(arr[l], arr[r]);
            ++l;
            --r;
        }
    } else {
        while (end < n && !(arr[end] < arr[end - 1])) {
            ++end;
        }
    }
    return end;
}

inline int node_power(size_t s1, size_t n1, size_t s2, size_t n2, size_t n) noexcept {
    uint64_t l = ((2ULL * static_cast<uint64_t>(s1) + static_cast<uint64_t>(n1)) << 30) / (2ULL * static_cast<uint64_t>(n));
    uint64_t r = ((2ULL * static_cast<uint64_t>(s2) + static_cast<uint64_t>(n2)) << 30) / (2ULL * static_cast<uint64_t>(n));
    uint64_t diff = l ^ r;
    if (diff == 0) return 31;
    return static_cast<int>(__builtin_clzll(diff << 34) + 1);
}

template <typename T>
void merge_adjacent_runs(T* arr, T* buf, size_t s1, size_t n1, size_t n2) {
    size_t mid = s1 + n1;
    size_t end = mid + n2;

    if (!(arr[mid] < arr[mid - 1])) return;

    if (n1 <= n2) {
        for (size_t i = 0; i < n1; ++i) {
            buf[i] = std::move(arr[s1 + i]);
        }
        size_t i = 0, j = mid, k = s1;
        while (i < n1 && j < end) {
            if (!(arr[j] < buf[i])) {
                arr[k++] = std::move(buf[i++]);
            } else {
                arr[k++] = std::move(arr[j++]);
            }
        }
        while (i < n1) {
            arr[k++] = std::move(buf[i++]);
        }
    } else {
        for (size_t i = 0; i < n2; ++i) {
            buf[i] = std::move(arr[mid + i]);
        }
        ptrdiff_t i = static_cast<ptrdiff_t>(mid) - 1;
        ptrdiff_t j = static_cast<ptrdiff_t>(n2) - 1;
        ptrdiff_t k = static_cast<ptrdiff_t>(end) - 1;
        while (i >= static_cast<ptrdiff_t>(s1) && j >= 0) {
            if (buf[j] < arr[i]) {
                arr[k--] = std::move(arr[i--]);
            } else {
                arr[k--] = std::move(buf[j--]);
            }
        }
        while (j >= 0) {
            arr[k--] = std::move(buf[j--]);
        }
    }
}

struct RunEntry {
    size_t start;
    size_t len;
    int power;
};

} // namespace powersort_detail

template <typename T>
void power_sort(std::vector<T>& arr) {
    RecursionGuard<T> rec_guard;
    const size_t n = arr.size();
    if (n < 2) return;

    size_t buf_cap = (n / 2) + 1;
    AuxMemoryGuard<T> mem_guard(buf_cap * sizeof(int32_t));
    std::vector<T> buf(buf_cap);

    T* data = arr.data();
    powersort_detail::RunEntry stack[64];
    size_t top = 0;

    size_t s1 = 0;
    size_t e1 = powersort_detail::extend_and_normalize_run(data, s1, n);
    size_t n1 = e1 - s1;
    if (n1 < powersort_detail::MIN_RUN_LEN) {
        e1 = std::min(n, s1 + powersort_detail::MIN_RUN_LEN);
        insertion_sort_range(data + s1, data + e1);
        n1 = e1 - s1;
    }

    while (e1 < n) {
        size_t s2 = e1;
        size_t e2 = powersort_detail::extend_and_normalize_run(data, s2, n);
        size_t n2 = e2 - s2;
        if (n2 < powersort_detail::MIN_RUN_LEN) {
            e2 = std::min(n, s2 + powersort_detail::MIN_RUN_LEN);
            insertion_sort_range(data + s2, data + e2);
            n2 = e2 - s2;
        }

        int p = powersort_detail::node_power(s1, n1, s2, n2, n);
        while (top > 0 && stack[top - 1].power > p) {
            --top;
            powersort_detail::merge_adjacent_runs(data, buf.data(), stack[top].start, stack[top].len, n1);
            s1 = stack[top].start;
            n1 += stack[top].len;
        }

        stack[top++] = {s1, n1, p};
        s1 = s2;
        e1 = e2;
        n1 = n2;
    }

    while (top > 0) {
        --top;
        powersort_detail::merge_adjacent_runs(data, buf.data(), stack[top].start, stack[top].len, n1);
        s1 = stack[top].start;
        n1 += stack[top].len;
    }
}

// 5. PDQSORT (Pattern-Defeating Quicksort)
namespace pdqsort_detail {

constexpr ptrdiff_t INSERTION_SORT_THRESHOLD = 24;
constexpr ptrdiff_t NINTHER_THRESHOLD = 128;
constexpr int PARTIAL_INSERTION_SORT_LIMIT = 8;

template <typename T>
inline void sort2(T* a, T* b) {
    if (*b < *a) alg_swap(*a, *b);
}

template <typename T>
inline void sort3(T* a, T* b, T* c) {
    sort2(a, b);
    sort2(b, c);
    sort2(a, b);
}

template <typename T>
void sift_down(T* begin, size_t start, size_t end) {
    size_t root = start;
    while (2 * root + 1 < end) {
        size_t child = 2 * root + 1;
        if (child + 1 < end && begin[child] < begin[child + 1]) {
            ++child;
        }
        if (begin[root] < begin[child]) {
            alg_swap(begin[root], begin[child]);
            root = child;
        } else {
            return;
        }
    }
}

template <typename T>
void heapsort_range(T* begin, T* end) {
    size_t count = static_cast<size_t>(end - begin);
    if (count < 2) return;

    for (ptrdiff_t start = static_cast<ptrdiff_t>(count / 2) - 1; start >= 0; --start) {
        sift_down(begin, static_cast<size_t>(start), count);
    }
    for (size_t last = count - 1; last > 0; --last) {
        alg_swap(begin[0], begin[last]);
        sift_down(begin, 0, last);
    }
}

template <typename T>
bool partial_insertion_sort(T* begin, T* end) {
    if (begin == end) return true;
    int limit = 0;

    for (T* cur = begin + 1; cur < end; ++cur) {
        if (*cur < *(cur - 1)) {
            T key = std::move(*cur);
            T* j = cur;
            do {
                *j = std::move(*(j - 1));
                --j;
            } while (j > begin && key < *(j - 1));
            *j = std::move(key);

            limit += static_cast<int>(cur - j);
            if (limit > PARTIAL_INSERTION_SORT_LIMIT) return false;
        }
    }
    return true;
}

template <typename T>
T* partition_left(T* begin, T* end) {
    T pivot = std::move(*begin);
    T* first = begin;
    T* last = end;

    while (pivot < *(--last)) {}

    if (last + 1 == end) {
        while (first < last && !(pivot < *(++first))) {}
    } else {
        while (!(pivot < *(++first))) {}
    }

    while (first < last) {
        alg_swap(*first, *last);
        while (pivot < *(--last)) {}
        while (!(pivot < *(++first))) {}
    }

    T* pivot_pos = last;
    *begin = std::move(*pivot_pos);
    *pivot_pos = std::move(pivot);
    return pivot_pos;
}

template <typename T>
std::pair<T*, bool> partition_right(T* begin, T* end) {
    T pivot = std::move(*begin);
    T* first = begin;
    T* last = end;

    while (*(++first) < pivot) {}

    if (first - 1 == begin) {
        while (first < last && !(*(--last) < pivot)) {}
    } else {
        while (!(*(--last) < pivot)) {}
    }

    bool already_partitioned = (first >= last);

    while (first < last) {
        alg_swap(*first, *last);
        while (*(++first) < pivot) {}
        while (!(*(--last) < pivot)) {}
    }

    T* pivot_pos = first - 1;
    *begin = std::move(*pivot_pos);
    *pivot_pos = std::move(pivot);
    return {pivot_pos, already_partitioned};
}

template <typename T>
void pdqsort_loop(T* begin, T* end, int bad_allowed, bool leftmost) {
    RecursionGuard<T> rec_guard;

    while (true) {
        ptrdiff_t size = end - begin;
        if (size < INSERTION_SORT_THRESHOLD) {
            insertion_sort_range(begin, end);
            return;
        }

        ptrdiff_t s2 = size / 2;
        if (size > NINTHER_THRESHOLD) {
            sort3(begin, begin + s2, end - 1);
            sort3(begin + 1, begin + (s2 - 1), end - 2);
            sort3(begin + 2, begin + (s2 + 1), end - 3);
            sort3(begin + (s2 - 1), begin + s2, begin + (s2 + 1));
            alg_swap(*begin, *(begin + s2));
        } else {
            sort3(begin + s2, begin, end - 1);
        }

        if (!leftmost && !(*(begin - 1) < *begin)) {
            begin = partition_left(begin, end) + 1;
            continue;
        }

        auto part_res = partition_right(begin, end);
        T* pivot_pos = part_res.first;
        bool already_partitioned = part_res.second;

        ptrdiff_t l_size = pivot_pos - begin;
        ptrdiff_t r_size = end - (pivot_pos + 1);
        bool highly_unbalanced = (l_size < size / 8) || (r_size < size / 8);

        if (highly_unbalanced) {
            if (--bad_allowed == 0) {
                heapsort_range(begin, end);
                return;
            }
            if (l_size >= INSERTION_SORT_THRESHOLD) {
                alg_swap(*begin, *(begin + l_size / 4));
                alg_swap(*(pivot_pos - 1), *(pivot_pos - l_size / 4));
                if (l_size > NINTHER_THRESHOLD) {
                    alg_swap(*(begin + 1), *(begin + (l_size / 4 + 1)));
                    alg_swap(*(begin + 2), *(begin + (l_size / 4 + 2)));
                    alg_swap(*(pivot_pos - 2), *(pivot_pos - (l_size / 4 + 1)));
                    alg_swap(*(pivot_pos - 3), *(pivot_pos - (l_size / 4 + 2)));
                }
            }
            if (r_size >= INSERTION_SORT_THRESHOLD) {
                alg_swap(*(pivot_pos + 1), *(pivot_pos + (1 + r_size / 4)));
                alg_swap(*(end - 1), *(end - r_size / 4));
                if (r_size > NINTHER_THRESHOLD) {
                    alg_swap(*(pivot_pos + 2), *(pivot_pos + (2 + r_size / 4)));
                    alg_swap(*(pivot_pos + 3), *(pivot_pos + (3 + r_size / 4)));
                    alg_swap(*(end - 2), *(end - (1 + r_size / 4)));
                    alg_swap(*(end - 3), *(end - (2 + r_size / 4)));
                }
            }
        } else {
            if (already_partitioned &&
                partial_insertion_sort(begin, pivot_pos) &&
                partial_insertion_sort(pivot_pos + 1, end)) {
                return;
            }
        }

        pdqsort_loop(begin, pivot_pos, bad_allowed, leftmost);
        begin = pivot_pos + 1;
        leftmost = false;
    }
}

inline int floor_log2(size_t n) noexcept {
    int log = 0;
    while (n >>= 1) ++log;
    return log;
}

} // namespace pdqsort_detail

template <typename T>
void pdq_sort(std::vector<T>& arr) {
    AuxMemoryGuard<T> mem_guard(0);
    if (arr.size() < 2) return;
    pdqsort_detail::pdqsort_loop(
        arr.data(),
        arr.data() + arr.size(),
        pdqsort_detail::floor_log2(arr.size()),
        true
    );
}

} // namespace bench

#endif // ALGORITHMS_HPP
