#include <progressive/result_struct.hpp>
#include <cstdint>
#include <stdio.h>
#include <assert.h>
#include <cstring>
#include <cstdlib>
#include "progressive/progressive_quicksort.hpp"
#include  <utility>
#define INSERT_SORT_LEVEL 64

using namespace std;
//! insertion sort
static void insertion_sort(int64_t *val, size_t *ind, int n) {
    int k;
    for (k = 1; k < n; k++) {
        int64_t key = val[k];
        size_t index = ind[k];
        int i = k - 1;
        while ((i >= 0) && (key < val[i])) {
            val[i + 1] = val[i];
            ind[i + 1] = ind[i];
            i--;
        }
        val[i + 1] = key;
        ind[i + 1] = index;
    }
}

//! hybrid radix sort: switches to insertion sort after a threshold
static void do_hybrid_radixsort_insert(int64_t *val, size_t *ind, unsigned int n, int shift) {
    unsigned int last[256], ptr[256], cnt[256];
    unsigned int i, j, k, sorted, remain;
    int64_t temp, swap;
    size_t temp_i, swap_i;

    memset(cnt, 0, 256 * sizeof(unsigned int)); //! Zero counters
    switch (shift) {                            //! Count occurrences
        case 0:
            for (i = 0; i < n; i++)
                cnt[val[i] & 0xFF]++;
            break;
        case 8:
            for (i = 0; i < n; i++)
                cnt[(val[i] >> 8) & 0xFF]++;
            break;
        case 16:
            for (i = 0; i < n; i++)
                cnt[(val[i] >> 16) & 0xFF]++;
            break;
        case 24:
            for (i = 0; i < n; i++)
                cnt[val[i] >> 24]++;
            break;
            /*
             * 	Note that even though our radix sort implementations work on 8 byte keys, only 4 bytes are
             * actually processed, as our key domain is never lager than [0, 2^32-1]. In a system
             * implementation, this would be handled before sorting by inspecting the max value of the
             * column in the statistics.
             */
    }
    sorted = (cnt[0] == n); //! Accumulate counters into pointers
    ptr[0] = 0;
    last[0] = cnt[0];
    for (i = 1; i < 256; i++) {
        last[i] = (ptr[i] = last[i - 1]) + cnt[i];
        sorted |= (cnt[i] == n);
    }
    if (!sorted) { //! Go through all swaps
        i = 255;
        remain = n;
        while (remain > 0) {
            while (ptr[i] == last[i])
                i--;    //! Find uncompleted value range
            j = ptr[i]; //! Grab first element in cycle
            swap = val[j];
            swap_i = ind[j];
            k = (swap >> shift) & 0xFF;
            if (i != k) { //! Swap into correct range until cycle completed
                do {
                    temp = val[ptr[k]];
                    temp_i = ind[ptr[k]];
                    ind[ptr[k]] = swap_i;
                    val[ptr[k]++] = swap;
                    swap = temp;
                    swap_i = temp_i;
                    k = (swap >> shift) & 0xFF;
                    remain--;
                } while (i != k);
                val[j] = swap; //! Place last element in cycle
                ind[j] = swap_i;
            }
            ptr[k]++;
            remain--;
        }
    }
    if (shift > 0) { //! Sort on next digit
        shift -= 8;
        for (i = 0; i < 256; i++) {
            if (cnt[i] > INSERT_SORT_LEVEL)
                do_hybrid_radixsort_insert(&val[last[i] - cnt[i]], &ind[last[i] - cnt[i]], cnt[i], shift);
            else if (cnt[i] > 1)
                insertion_sort(&val[last[i] - cnt[i]], &ind[last[i] - cnt[i]], cnt[i]);
        }
    }
}

bool itqs(int64_t *val, size_t *ind, size_t n) {
    do_hybrid_radixsort_insert(val, ind, n, 24);
    return true;
}

int64_t binary_search(int64_t *c, int64_t key, int64_t lower, int64_t upper, bool *foundKey) {

    *foundKey = false;
    upper--;
    while (lower <= upper) {
        int middle = (lower + upper) / 2;
        auto middleElement = c[middle];

        if (middleElement < key) {
            lower = middle + 1;
        } else if (middleElement > key) {
            upper = middle - 1;
        } else {
            *foundKey = true;
            return middle;
        }
    }
    return upper;
}


int64_t binary_search_gte(int64_t *c, int64_t key, int64_t start, int64_t end) {
    bool found = false;
    int pos = binary_search(c, key, start, end, &found);
    if (found) {
        while (--pos >= start && c[pos] == key);
    }
    ++pos;
    return pos;
}

int64_t binary_search_lte(int64_t *c, int64_t key, int64_t start, int64_t end) {
    bool found = false;
    int pos = binary_search(c, key, start, end, &found);
    while (pos < end && c[pos] <= key)
        pos++;
    pos--;

    return pos;
}

int64_t binary_search(vector<pair<int64_t,size_t>>& c, int64_t key, int64_t lower, int64_t upper, bool *foundKey) {

    *foundKey = false;
    upper--;
    while (lower <= upper) {
        int middle = (lower + upper) / 2;
        auto middleElement = c[middle].first;

        if (middleElement < key) {
            lower = middle + 1;
        } else if (middleElement > key) {
            upper = middle - 1;
        } else {
            *foundKey = true;
            return middle;
        }
    }
    return upper;
}


int64_t binary_search_gte(vector<pair<int64_t,size_t>>& c, int64_t key, int64_t start, int64_t end) {
    bool found = false;
    int pos = binary_search(c, key, start, end, &found);
    if (found) {
        while (--pos >= start && c[pos].first == key);
    }
    ++pos;
    return pos;
}

int64_t binary_search_lte(vector<pair<int64_t,size_t>>& c, int64_t key, int64_t start, int64_t end) {
    bool found = false;
    int pos = binary_search(c, key, start, end, &found);
    while (pos < end && c[pos].first <= key)
        pos++;
    pos--;

    return pos;
}


void ProgressiveQuicksort::create_phase(std::pair<int64_t, int64_t> range_query, ResultStruct &results,
                                        ssize_t &remaining_swaps) {
    int64_t *index = qs_index.data;
    size_t *pointers = qs_index.index;

    //! for the initial run, we write the indices instead of swapping them
    //! because the current array has not been initialized yet
    QuicksortNode &node = qs_index.root;
    //! first look through the part we have already pivoted
    //! for data that matches the points
    if (range_query.first < node.pivot) {
        for (size_t i = 0; i < node.current_start; i++) {
            int matching = index[i] >= range_query.first && index[i] < range_query.second;
            results.maybe_push_back(index[i], matching);
        }
    }
    if (range_query.second >= node.pivot) {
        for (size_t i = node.current_end + 1; i < original_column->size(); i++) {
            int matching = index[i] >= range_query.first && index[i] < range_query.second;
            results.maybe_push_back(index[i], matching);
        }
    }
    //! now we start filling the index with at most remaining_swap entries
    size_t next_index = std::min(qs_index.current_position + remaining_swaps, original_column->size());
    remaining_swaps -= next_index - qs_index.current_position;
    for (size_t i = qs_index.current_position; i < next_index; i++) {
        int matching = (*original_column)[i] >= range_query.first && (*original_column)[i] < range_query.second;
        results.maybe_push_back((*original_column)[i], matching);

        int bigger_pivot = (*original_column)[i] >= node.pivot;
        int smaller_pivot = 1 - bigger_pivot;

        index[node.current_start] = (*original_column)[i];
        index[node.current_end] = (*original_column)[i];
        pointers[node.current_start] = i;
        pointers[node.current_end] = i;

        node.current_start += smaller_pivot;
        node.current_end -= bigger_pivot;
    }
    qs_index.current_position = next_index;
    if (next_index == original_column->size()) {
        //! we are finished with the initial run
        //! construct the left and right side of the root node
        QuicksortNode left(0);
        left.start = node.start;
        left.end = index[node.current_start] < node.pivot ? node.current_start + 1 : node.current_start;
        left.current_start = left.start;
        left.current_end = left.end - 1;
        left.pivot = (index[left.current_start] + index[left.current_end]) / 2;
        left.min = node.min;
        left.max = node.pivot;
        QuicksortNode right(1);
        right.start = left.end;
        right.end = node.end;
        right.current_start = right.start;
        right.current_end = right.end - 1;
        right.pivot = (index[right.current_start] + index[right.current_end]) / 2;
        right.min = node.pivot;
        right.max = node.max;
        qs_index.root.left = qs_index.nodes.size();
        qs_index.nodes.push_back(left);
        qs_index.root.right = qs_index.nodes.size();
        qs_index.nodes.push_back(right);
        qs_index.current_pivot = 0;
    } else {
        //! we have done all the swapping for this run
        //! now we query the remainder of the data

        for (size_t i = qs_index.current_position; i < original_column->size(); i++) {
            int matching = (*original_column)[i] >= range_query.first && (*original_column)[i] < range_query.second;
            results.maybe_push_back((*original_column)[i], matching);
        }
    }
}

void range_query_sorted_subsequent_value(int64_t *index, size_t index_size, int64_t low, int64_t high, int64_t min,
                                         int64_t max, ResultStruct &results) {
    if (low <= min) {
        if (high > max) {
            //! just add all the elements
            for (size_t i = 0; i < index_size; i++) {
                results.push_back(index[i]);
            }
        } else {
            //! no need for binary search to obtain first entry
            //! first entry is 0
            for (size_t i = 0; i < index_size; i++) {
                if (index[i] < high) {
                    results.push_back(index[i]);
                } else {
                    break;
                }
            }
        }
    } else {
        //! perform binary search to find first element
        auto entry = &index[binary_search_gte(index, low, 0, index_size)];
        if (high > max) {
            //! no need for check after binary search
            for (; entry != index + index_size; entry++) {
                results.push_back(*entry);
            }
        } else {
            for (; entry != index + index_size; entry++) {
                if (*entry < high) {
                    results.push_back(*entry);
                } else {
                    break;
                }
            }
        }
    }
}

void ProgressiveQuicksort::SortedCheck(QuicksortNode &node) {
    if (qs_index.nodes[node.left].sorted && qs_index.nodes[node.right].sorted) {
        node.sorted = true;
        node.left = -1;
        node.right = -1;
        if (node.position >= 0) {
            if (node.parent >= 0) {
                SortedCheck(qs_index.nodes[node.parent]);
            } else {
                SortedCheck(qs_index.root);
            }
        }
    }
}

void ProgressiveQuicksort::refine_phase(QuicksortNode &node, ResultStruct &results, int64_t low,
                                        int64_t high, ssize_t &remaining_swaps) {
    int64_t *index = qs_index.data;
    size_t *pointers = qs_index.index;
    if (node.sorted) {
        if (low <= node.min && high > node.max) {
            //! query contains entire node, just add all the entries to the result
            for (size_t i = node.start; i < node.end; i++) {
                results.push_back(index[i]);
            }
        } else {
            range_query_sorted_subsequent_value(qs_index.data + node.start, node.end - node.start, low, high,
                                                node.min, node.max, results);
        }
        return;
    }

    if (node.left < 0) {
        if (node.min == node.max) {
            node.sorted = true;
            SortedCheck(node.parent >= 0 ? qs_index.nodes[node.parent] : qs_index.root);
            range_query_sorted_subsequent_value(qs_index.data + node.start, node.end - node.start, low, high,
                                                node.min, node.max, results);
            return;
        } else if ((node.end - node.start) <= 1024) {
            //! node is very small, just sort it normally
            if (remaining_swaps > (node.end - node.start) * 5) {
                itqs(qs_index.data + node.start, qs_index.index + node.start, node.end - node.start);
                node.sorted = true;
                SortedCheck(node.parent >= 0 ? qs_index.nodes[node.parent] : qs_index.root);
                remaining_swaps -= (node.end - node.start) * 5; //! log2(8192)
                range_query_sorted_subsequent_value(qs_index.data + node.start, node.end - node.start, low, high,
                                                    node.min, node.max, results);
            } else {
                //! node is small but we don't have enough swaps left to sort
                //! scan to add to result
                for (size_t i = node.start; i < node.end; i++) {
                    int matching = index[i] >= low && index[i] < high;
                    results.maybe_push_back(index[i], matching);
                }
            }
            return;
        }
        //! we pivot again here
        //! first if we have already done some swaps
        //! look into the parts we have swapped already
        if (low < node.pivot) {
            for (size_t i = node.start; i < node.current_start; i++) {
                int matching = index[i] >= low && index[i] < high;
                results.maybe_push_back(index[i], matching);
            }
        }
        if (high >= node.pivot) {
            for (size_t i = node.current_end + 1; i < node.end; i++) {
                int matching = index[i] >= low && index[i] < high;
                results.maybe_push_back(index[i], matching);
            }
        }

        auto old_start = node.current_start;
        auto old_end = node.current_end;

        //! now we crack some pieces baby
        while (node.current_start < node.current_end && remaining_swaps > 0) {
            int64_t start = index[node.current_start];
            int64_t end = index[node.current_end];
            size_t start_pointer = pointers[node.current_start];
            size_t end_pointer = pointers[node.current_end];

            int start_has_to_swap = start >= node.pivot;
            int end_has_to_swap = end < node.pivot;
            int has_to_swap = start_has_to_swap * end_has_to_swap;

            index[node.current_start] = !has_to_swap * start + has_to_swap * end;
            index[node.current_end] = !has_to_swap * end + has_to_swap * start;
            pointers[node.current_start] = !has_to_swap * start_pointer + has_to_swap * end_pointer;
            pointers[node.current_end] = !has_to_swap * end_pointer + has_to_swap * start_pointer;

            node.current_start += !start_has_to_swap + has_to_swap;
            node.current_end -= !end_has_to_swap + has_to_swap;
            remaining_swaps--;
        }

        //! scan the remainder (if any)
        for (size_t i = old_start; i <= old_end; i++) {
            int matching = index[i] >= low && index[i] < high;
            results.maybe_push_back(index[i], matching);
        }
        if (node.current_start >= node.current_end) {
            if (node.current_start == node.start || node.current_end == node.end - 1) {
                //! either the left or right side would have zero entries
                //! this means either (1) the pivot was chosen poorly
                //!                or (2) the whole chunk only contains the same value
                int64_t old_pivot = node.pivot;
                //! otherwise in (1) we pivot again, but move the pivot closer to the min or max
                if (node.current_start == node.start) {
                    assert(node.pivot != node.max);
                    node.pivot = node.pivot / 2 + node.max / 2;
                } else {
                    assert(node.pivot != node.min);
                    node.pivot = node.pivot / 2 + node.min / 2;
                }
                if (node.pivot == old_pivot) {
                    //! everything apparently has the same value
                    node.sorted = true;
                    SortedCheck(node.parent >= 0 ? qs_index.nodes[node.parent] : qs_index.root);
                    return;
                }
                node.current_start = node.start;
                node.current_end = node.end - 1;
                return;
            }

            node.left = qs_index.nodes.size();
            QuicksortNode left(node.left, node.position);
            left.start = node.start;
            left.end = index[node.current_start] < node.pivot ? node.current_start + 1 : node.current_start;
            left.current_start = left.start;
            left.current_end = left.end - 1;
            left.min = node.min;
            left.max = node.pivot;
            left.pivot = (index[left.current_start] + index[left.current_end]) / 2;
            node.right = qs_index.nodes.size() + 1;
            QuicksortNode right(node.right, node.position);
            right.start = left.end;
            right.end = node.end;
            right.current_start = right.start;
            right.current_end = right.end - 1;
            right.min = node.pivot;
            right.max = node.max;
            right.pivot = (index[right.current_start] + index[right.current_end]) / 2;
            qs_index.nodes.push_back(left);
            qs_index.nodes.push_back(right);
        }
        return;
    } else {
        auto left = node.left;
        auto right = node.right;
        auto pivot = node.pivot;
        assert(left >= 0 && right >= 0);
        //! node has children, go into one of the children
        if (low < pivot) {
            refine_phase(qs_index.nodes[left], results, low, high, remaining_swaps);
        }
        if (high >= pivot) {
            refine_phase(qs_index.nodes[right], results, low, high,
                         remaining_swaps);
        }
    }
}

void range_query_sorted_subsequent_value(int64_t *index, size_t index_size, int64_t low, int64_t high,
                                         ResultStruct &results) {
    int64_t lower_bound = binary_search_gte(index, low, 0, index_size);
    int64_t high_bound = binary_search_lte(index, high - 1, 0, index_size);
    for (int64_t i = lower_bound; i <= high_bound; i++) {
        results.push_back(index[i]);
    }
}

void range_query_sorted_subsequent_value(vector<pair<int64_t,size_t>> &index, size_t index_size, int64_t low, int64_t high,
                                         ResultStruct &results) {
    int64_t lower_bound = binary_search_gte(index, low, 0, index_size);
    int64_t high_bound = binary_search_lte(index, high - 1, 0, index_size);
    for (int64_t i = lower_bound; i <= high_bound; i++) {
        results.push_back(index[i].first);
    }
}

ResultStruct
ProgressiveQuicksort::execute_range_query(std::pair<int64_t, int64_t> range_query, ssize_t &remaining_swaps) {
    ResultStruct results;
    if (qs_index.root.sorted) {
        if (!qs_index.data){
            //! array is sorted entirely already
        range_query_sorted_subsequent_value(original_column->data, original_column->size(), range_query.first,
                                            range_query.second, results);
        }else{
            //! array is sorted entirely already
        range_query_sorted_subsequent_value(qs_index.data, original_column->size(), range_query.first,
                                            range_query.second, results);
        }
        return results;
    }

    if (!qs_index.index) {
        //! fill the initial sortindex
        //! choose an initial pivot point
        qs_index.index = (size_t *) malloc(sizeof(size_t) * original_column->size());
        qs_index.data = (int64_t *) malloc(sizeof(int64_t) * original_column->size());
        qs_index.root.pivot = ((*original_column)[0] + (*original_column)[original_column->size() - 1]) / 2;
        qs_index.root.start = 0;
        qs_index.root.end = original_column->size();
        qs_index.root.current_start = qs_index.root.start;
        qs_index.root.current_end = qs_index.root.end - 1;
        qs_index.root.min = INT_MIN;
        qs_index.root.max = INT_MAX;
        qs_index.current_position = 0;
        qs_index.size = original_column->size();
    }

    if (qs_index.root.left < 0) {
        create_phase(range_query, results, remaining_swaps);
    } else {
        refine_phase(qs_index.root, results, range_query.first, range_query.second, remaining_swaps);
    }
    while (remaining_swaps > 0 && qs_index.current_pivot < qs_index.nodes.size()) {
        QuicksortNode &node = qs_index.nodes[qs_index.current_pivot];
        if (node.sorted || node.left >= 0) {
            qs_index.current_pivot++;
            continue;
        }
        if (node.min == node.max) {
            node.sorted = true;
            SortedCheck(node.parent >= 0 ? qs_index.nodes[node.parent] : qs_index.root);
            qs_index.current_pivot++;
        } else if ((node.end - node.start) <= 1024) {
            if (remaining_swaps < (node.end - node.start) * 5) {
                //! not enough swaps left to sort
                break;
            }
            //! node is very small, just sort it normally
            itqs(qs_index.data + node.start, qs_index.index + node.start, node.end - node.start);
            node.sorted = true;
            SortedCheck(node.parent >= 0 ? qs_index.nodes[node.parent] : qs_index.root);
            remaining_swaps -= (node.end - node.start) * 5; //! log2(8192)
            qs_index.current_pivot++;
        } else {
            int64_t *index = qs_index.data;
            size_t *pointers = qs_index.index;
            while (node.current_start < node.current_end && remaining_swaps > 0) {
                int64_t start = index[node.current_start];
                int64_t end = index[node.current_end];
                size_t start_pointer = pointers[node.current_start];
                size_t end_pointer = pointers[node.current_end];

                int start_has_to_swap = start >= node.pivot;
                int end_has_to_swap = end < node.pivot;
                int has_to_swap = start_has_to_swap * end_has_to_swap;

                index[node.current_start] = !has_to_swap * start + has_to_swap * end;
                index[node.current_end] = !has_to_swap * end + has_to_swap * start;
                pointers[node.current_start] = !has_to_swap * start_pointer + has_to_swap * end_pointer;
                pointers[node.current_end] = !has_to_swap * end_pointer + has_to_swap * start_pointer;

                node.current_start += !start_has_to_swap + has_to_swap;
                node.current_end -= !end_has_to_swap + has_to_swap;
                remaining_swaps--;
            }
            if (node.current_start >= node.current_end) {
                if (node.current_start == node.start || node.current_end == node.end - 1) {
                    //! either the left or right side would have zero entries
                    //! this means either (1) the pivot was chosen poorly
                    //!                or (2) the whole chunk only contains the same value
                    //! first check for (2)
                    //! otherwise in (1) we pivot again, but move the pivot closer to the min or max
                    remaining_swaps -= node.end - node.start;
                    if (node.current_start == node.start) {
                        assert(node.pivot != node.max);
                        node.pivot = (node.pivot + node.max) / 2;
                    } else {
                        assert(node.pivot != node.min);
                        node.pivot = (node.pivot + node.min) / 2;
                    }
                    node.current_start = node.start;
                    node.current_end = node.end - 1;
                    continue;
                }
                node.left = qs_index.nodes.size();
                QuicksortNode left(node.left, node.position);
                left.start = node.start;
                left.end = index[node.current_start] < node.pivot ? node.current_start + 1 : node.current_start;
                left.current_start = left.start;
                left.current_end = left.end - 1;
                left.min = node.min;
                left.max = node.pivot;
                left.pivot = (index[left.current_start] + index[left.current_end]) / 2;
                node.right = qs_index.nodes.size() + 1;
                QuicksortNode right(node.right, node.position);
                right.start = left.end;
                right.end = node.end;
                right.current_start = right.start;
                right.current_end = right.end - 1;
                right.pivot = (index[right.current_start] + index[right.current_end]) / 2;
                right.min = node.pivot;
                right.max = node.max;
                qs_index.nodes.push_back(left);
                qs_index.nodes.push_back(right);
                qs_index.current_pivot++;
            }
        }
    }
    return results;
}