#include <cstdlib>
#include <assert.h>
#include <cstring>
#include "progressive/progressive_index.hpp"

void ProgressiveIndex::build(Column &original_column, std::pair<int64_t, int64_t> range_query) {
    //! Nothing to do
}
void ProgressiveIndex::search(Column &original_column, std::pair<int64_t, int64_t> range_query,
                              int64_t &result) {
    auto remaining_swaps = (ssize_t)(original_column.size() * delta);
    auto res_og = column_index->execute_range_query(range_query, remaining_swaps).sum;
    result = res_og + update_index.execute_range_query(original_column,range_query, remaining_swaps).sum;
}

void ProgressiveIndex::push_update(std::pair<int64_t, int64_t> update) {
    update_index.updates->push_back(update.first, update.second);
}