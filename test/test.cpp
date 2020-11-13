#define CATCH_CONFIG_MAIN //! This tells Catch to provide a main() - only do
                          //! this in one cpp file

#include "catch.hpp"
#include <experiment.hpp>
size_t column_size = 1000000;
size_t num_updates = 100;
size_t frequency = 10;

TEST_CASE("Complete", "[cracking]") {
  Experiment experiment(column_size,num_updates,frequency,IndexType::Cracking,UpdateType::Complete);
  for (size_t i {}; i < experiment.NUM_QUERIES; i ++){
    REQUIRE(experiment.execute_query(i) == experiment.execute_query_fs(i));
  }
}

TEST_CASE("Gradual", "[cracking]") {
  Experiment experiment(column_size,num_updates,frequency,IndexType::Cracking,UpdateType::Gradual);
  for (size_t i {}; i < experiment.NUM_QUERIES; i ++){
    REQUIRE(experiment.execute_query(i) == experiment.execute_query_fs(i));
  }
}

TEST_CASE("Ripple", "[cracking]") {
  Experiment experiment(column_size,num_updates,frequency,IndexType::Cracking,UpdateType::Ripple);
  for (size_t i {}; i < experiment.NUM_QUERIES; i ++){
    REQUIRE(experiment.execute_query(i) == experiment.execute_query_fs(i));
  }
}