#include "cracking/cracker_index.hpp"
#include "include/column.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include "experiment.hpp"
using namespace std;
using namespace chrono;


void print_help(int argc, char **argv) {
	fprintf(stderr, "Unrecognized command line option.\n");
	fprintf(stderr, "Usage: %s [args]\n", argv[0]);
	fprintf(stderr, "   --num-queries\n");
	fprintf(stderr, "   --column-size\n");
	fprintf(stderr, "   --index-type\n");
	fprintf(stderr, "   --update-type\n");
	fprintf(stderr, "   --delta\n");
	fprintf(stderr, "   --append-size\n");
	fprintf(stderr, "   --append-frequency\n");
	fprintf(stderr, "   --updates-after\n");
}

pair<string, string> split_once(string delimited, char delimiter) {
	auto pos = delimited.find_first_of(delimiter);
	return {delimited.substr(0, pos), delimited.substr(pos + 1)};
}

UpdateType get_update_type(size_t type){
    switch (type) {
        case 1:
            return UpdateType::Complete;
        case 2:
            return UpdateType::Gradual;
        case 3:
            return UpdateType::Ripple;
        case 4:
            return UpdateType::Mergesort;
        default:
            throw "Update Type does not exist";
    }
}

IndexType get_index_type(size_t type){
    switch (type) {
        case 1:
            return IndexType::Cracking;
        case 2:
            return IndexType::Progressive;
        default:
            throw "Index Type does not exist";
    }
}

int main(int argc, char **argv) {
    size_t NUM_QUERIES = 1000 ,COLUMN_SIZE = 1000000,INDEX_TYPE = 1,UPDATE_TYPE = 1,APPEND_SIZE = 10,APPEND_FREQUENCY = 10, START_UPDATES_AFTER = 50;
    double DELTA = 0.1;
  	for (int i = 1; i < argc; i++) {
		auto arg = string(argv[i]);
		if (arg.substr(0, 2) != "--") {
			print_help(argc, argv);
			exit(EXIT_FAILURE);
		}
		arg = arg.substr(2);
		auto p = split_once(arg, '=');
		auto &arg_name = p.first;
		auto &arg_value = p.second;
		if (arg_name == "num-queries") {
			NUM_QUERIES = atoi(arg_value.c_str());
		} else if (arg_name == "column-size") {
			COLUMN_SIZE = atoi(arg_value.c_str());
		} else if (arg_name == "index-type") {
			INDEX_TYPE = atoi(arg_value.c_str());
		} else if (arg_name == "update-type") {
			UPDATE_TYPE = atoi(arg_value.c_str());
		} else if (arg_name == "delta") {
			DELTA = atof(arg_value.c_str());
		} else if (arg_name == "append-size") {
			APPEND_SIZE = atoi(arg_value.c_str());
		} else if (arg_name == "append-frequency") {
			APPEND_FREQUENCY = atoi(arg_value.c_str());
		} else if (arg_name == "updates-after") {
			START_UPDATES_AFTER = atoi(arg_value.c_str());
		}else {
			print_help(argc, argv);
			exit(EXIT_FAILURE);
		}
	}
  	Experiment experiment(NUM_QUERIES,COLUMN_SIZE,get_index_type(INDEX_TYPE),get_update_type(UPDATE_TYPE),APPEND_SIZE,APPEND_FREQUENCY,START_UPDATES_AFTER,DELTA);
  	experiment.run();
}
