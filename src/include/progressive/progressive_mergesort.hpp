#pragma once

#include <column.hpp>
#include <stdio.h>
#include "result_struct.hpp"
#include "progressive_quicksort.hpp"

class ProgressiveMergesort{
public:
    ResultStruct execute_range_query(Column &original_column, std::pair<int64_t, int64_t> range_query, ssize_t& remaining_swaps);
    Column *updates = new Column();

private:
    Column *merge_column = nullptr;
    double DELTA = 0.1;
    std::vector<ProgressiveQuicksort *> sort_chunks;
    size_t left_column = 0, right_column = 0, merge_index = 0;
    ssize_t left_chunk, right_chunk;
    std::vector<bool> has_converged;
    int unsorted_column_count = 1;
    double SORTED_COLUMN_RATIO = 16;
};