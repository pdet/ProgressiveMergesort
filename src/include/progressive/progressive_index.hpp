#pragma once

#include <index.hpp>
#include <climits>
#include <memory>
#include "result_struct.hpp"
#include "progressive_quicksort.hpp"
#include "progressive_mergesort.hpp"

class ProgressiveIndex : public Index {
public:
    ~ProgressiveIndex() override = default;

    explicit ProgressiveIndex(Column &original_column, double delta, UpdateType u_type) : delta(delta), update_type(u_type) {
        if (update_type != UpdateType::Mergesort) {
            throw "Update Type not Implemented for Progressive Indexing";
        }
        column_index = std::make_unique<ProgressiveQuicksort>(&original_column);
    };

    void build(Column &original_column, std::pair<int64_t, int64_t> range_query) override;

    void search(Column &original_column, std::pair<int64_t, int64_t> range_query,
                int64_t &result) override;

    void push_update(std::pair<int64_t, int64_t> update) override;

private:
    std::unique_ptr <ProgressiveQuicksort> column_index;
    UpdateType update_type;
    ProgressiveMergesort update_index;
    double delta = 0.1;
    bool converged = false;
};