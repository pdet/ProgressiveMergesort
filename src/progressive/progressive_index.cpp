#include <cstdlib>
#include <assert.h>
#include <cstring>
#include "progressive/progressive_index.hpp"

void ProgressiveIndex::build(Column &original_column, std::pair<int64_t, int64_t> range_query) {
    //! Nothing to do
}
void ProgressiveIndex::search(Column &original_column, std::pair<int64_t, int64_t> range_query,
                              int64_t &result) {
    double original_delta = update_index.unsorted_column_count == 0 ? 0 : delta / update_index.unsorted_column_count;
    auto remaining_swaps = (ssize_t)(original_column.size() * original_delta);
    auto res_og = column_index->execute_range_query(range_query, remaining_swaps).sum;
    if (!converged && column_index->qs_index.root.sorted){
        converged = true;
        update_index.unsorted_column_count--;
    }
    result = res_og + update_index.execute_range_query(original_column,range_query, remaining_swaps,original_delta).sum;
}

void ProgressiveIndex::push_update(std::pair<int64_t, int64_t> update) {
    update_index.updates->push_back(update.first, update.second);
}