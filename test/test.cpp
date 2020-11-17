#define CATCH_CONFIG_MAIN //! This tells Catch to provide a main() - only do
//! this in one cpp file

#include "catch.hpp"
#include <experiment.hpp>

size_t column_size = 1000000;
size_t num_updates = 10000;
size_t frequency = 10;

void run_experiment(IndexType i_type, UpdateType u_type) {
    Experiment experiment(column_size, num_updates, frequency, i_type, u_type);
    for (size_t i{}; i < experiment.NUM_QUERIES; i++) {
        std::cout << i << std::endl;
        int64_t result = experiment.execute_query(i);
        REQUIRE(result == experiment.execute_query_fs(i));
    }
}

TEST_CASE("Complete", "[cracking]") {
    run_experiment(IndexType::Cracking,UpdateType::Complete);
}

TEST_CASE("Gradual", "[cracking]") {
    run_experiment(IndexType::Cracking,UpdateType::Gradual);
}

TEST_CASE("Ripple", "[cracking]") {
    run_experiment(IndexType::Cracking,UpdateType::Ripple);
}

TEST_CASE("Mergesort", "[progressive]") {
    run_experiment(IndexType::Progressive,UpdateType::Mergesort);
}