#include <progressive/progressive_mergesort.hpp>
#include <progressive/result_struct.hpp>
#include <column.hpp>
#include <stdio.h>

ResultStruct ProgressiveMergesort::execute_range_query(Column &original_column, std::pair<int64_t, int64_t> range_query,
                                                       ssize_t &remaining_swaps, double original_delta) {
    ResultStruct results;

    for (size_t j = 0; j < sort_chunks.size(); j++) {
        auto &chunk = sort_chunks[j];
        double sort_chunk_delta = original_delta * ((double) original_column.size() / (double) chunk->qs_index.size);
        auto remaining_swaps_chunk = (ssize_t) (original_column.size() * sort_chunk_delta);
        results.merge(chunk->execute_range_query(range_query, remaining_swaps_chunk));
        if (!has_converged[j] && chunk->qs_index.root.sorted) {
            has_converged[j] = true;
            unsorted_column_count--;
        }
    }
    for (auto &element : updates->data) {
        int matching = element.first >= range_query.first && element.first < range_query.second;
        results.maybe_push_back(element.first, matching);
    }
    if (!merge_column && unsorted_column_count == 0 && sort_chunks.size() > 1) {
        // start a merge
        merge_column = new Column();
        left_chunk = sort_chunks.size() - 2;
        right_chunk = sort_chunks.size() - 1;
        merge_column->data.reserve(sort_chunks[left_chunk]->qs_index.size + sort_chunks[right_chunk]->qs_index.size);
        left_column = 0;
        right_column = 0;
        merge_index = 0;
    }
    if (merge_column) {
        size_t todo_merge = std::min(merge_column->data.capacity() - merge_index, (unsigned long) (DELTA * original_column.size()));
        for (size_t j = 0; j < todo_merge; j++) {
            if (left_column < sort_chunks[left_chunk]->qs_index.size &&
                (right_column >= sort_chunks[right_chunk]->qs_index.size ||
                  sort_chunks[left_chunk]->qs_index.data[left_column] < sort_chunks[right_chunk]->qs_index.data[right_column])) {
                merge_column->data.emplace_back(sort_chunks[left_chunk]->qs_index.data[left_column],sort_chunks[left_chunk]->qs_index.index[left_column]);
                left_column++;
//                merge_column->data[merge_index].first = sort_chunks[left_chunk]->qs_index.data[left_column];
//                merge_column->data[merge_index++].second = sort_chunks[left_chunk]->qs_index.index[left_column++];
            } else {
                merge_column->data.emplace_back(sort_chunks[right_chunk]->qs_index.data[right_column],sort_chunks[right_chunk]->qs_index.index[right_column]);
                right_column++;
//                merge_column->data[merge_index].first = sort_chunks[right_chunk]->qs_index.data[right_column];
//                merge_column->data[merge_index++].second = sort_chunks[right_chunk]->qs_index.index[right_column++];
            }
            merge_index++;
        }
        if (merge_index == merge_column->data.capacity()) {
            //! finish merging
            delete sort_chunks[left_chunk];
            delete sort_chunks[right_chunk];
            sort_chunks.erase(sort_chunks.begin() + left_chunk, sort_chunks.begin() + right_chunk + 1);
            auto *quickie = new ProgressiveQuicksort(merge_column);
            quickie->qs_index.root.sorted = true;
            sort_chunks.insert(sort_chunks.begin(), quickie);
            merge_column = nullptr;
        }
    }
    if (updates->size() > original_column.size() / SORTED_COLUMN_RATIO) {
        auto *quickie = new ProgressiveQuicksort(updates);
        //! start a quicksort on the updates
        sort_chunks.push_back(quickie);
        has_converged.push_back(false);
        updates = new Column();
        unsorted_column_count++;
    }
    return results;
}