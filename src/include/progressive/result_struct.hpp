#pragma once
#include <cstdint>
#include <cstddef>

struct ResultStruct {
    int64_t sum = 0;

    static void reserve(size_t capacity) {
        (void) capacity;
    }

    static size_t size() {
        return 1;
    }

    int64_t *begin() {
        return &sum;
    }

    int64_t *end() {
        return &sum + 1;
    }

    inline void push_back(int64_t value) {
        sum += value;
    }

    inline void maybe_push_back(int64_t value, int maybe) {
        sum += maybe * value;
    }

    inline void merge(ResultStruct other) {
        sum += other.sum;
    }

    int64_t &operator[](const size_t index) {
        return sum;
    }

    int64_t operator[](const size_t index) const {
        return sum;
    }

    ResultStruct() : sum(0) {
    }
};
