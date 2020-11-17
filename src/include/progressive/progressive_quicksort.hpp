#pragma once

#include <column.hpp>
#include <climits>

struct QuicksortNode {
	int64_t pivot;
	int64_t min = INT_MIN;
	int64_t max = INT_MAX;
	size_t start;
	size_t end;
	size_t current_start, current_end;
	bool sorted;

	int64_t position;
	int64_t parent;
	int64_t left;
	int64_t right;

	QuicksortNode() : position(-1), parent(-1), left(-1), right(-1), sorted(false), min(INT_MIN), max(INT_MAX) {
	}

	QuicksortNode(int64_t position)
	    : position(position), parent(-1), left(-1), right(-1), sorted(false), min(INT_MIN), max(INT_MAX) {
	}

	QuicksortNode(int64_t position, int64_t parent)
	    : position(position), parent(parent), left(-1), right(-1), sorted(false), min(INT_MIN), max(INT_MAX) {
	}
};

struct IncrementalQuicksortIndex {
	std::vector<QuicksortNode> nodes;
	QuicksortNode root;
	size_t *index = nullptr;
	int64_t *data = nullptr;
	size_t current_position = 0;
	size_t current_pivot = 0;
    size_t size;
	IncrementalQuicksortIndex() : index(nullptr), data(nullptr), current_pivot(0) {
	}
};
class ProgressiveQuicksort{

public:
    explicit ProgressiveQuicksort(Column* original_column): original_column(original_column){
        qs_index.size = original_column->size();
    };
    ResultStruct execute_range_query(std::pair<int64_t, int64_t> range_query, ssize_t &remaining_swaps);
    bool converged = false;
    IncrementalQuicksortIndex qs_index;
    Column *original_column{};
private:

    void create_phase(std::pair<int64_t, int64_t> range_query, ResultStruct &results, ssize_t &remaining_swaps);
    void refine_phase(QuicksortNode &node, ResultStruct &results, int64_t low,
                                                 int64_t high, ssize_t &remaining_swaps);
    void SortedCheck( QuicksortNode &node);
};