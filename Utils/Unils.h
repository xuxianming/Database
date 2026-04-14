#include <cstddef>
#include <numeric>
#ifndef UTILS_UNILS_H
#define UTILS_UNILS_H

template <size_t... Args>
struct LCM;

template <size_t A, size_t B>
struct LCM<A, B> {
    static constexpr size_t value = A * B / std::gcd(A, B);
};

template <size_t A, size_t B, size_t... Rest>
struct LCM<A, B, Rest...> {
    static constexpr size_t value = LCM<LCM<A, B>::value, Rest...>::value;
};

template <size_t N>
struct LCMRange {
    template <size_t... Is>
    static constexpr size_t compute(std::index_sequence<Is...>) {
        return LCM<Is...>::value;
    }
    static constexpr size_t value = compute(std::make_index_sequence<N + 1>());
};

static constexpr bool IsPerfectSize(size_t n) { return (n & (n + 1)) == 0; }

static constexpr size_t NextPerfectSize(size_t target) {
    size_t k = 1;
    size_t v = 1;
    while (v < target) {
        ++k;
        v = (1ULL << k) - 1;
    }
    return v;
}

#endif  // UTILS_UNILS_H