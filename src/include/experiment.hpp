#pragma once
#include "original_table.hpp"
#include "cracking/cracker_index.hpp"
enum IndexType { Cracking, Progressive };
class Experiment{
public:
  Experiment(size_t column_size, size_t num_updates, size_t frequency, IndexType index_type, UpdateType update_type);
  ~Experiment()= default;
  size_t NUM_UPDATES = 0;
  size_t COLUMN_SIZE = 10000000;
  size_t FREQUENCY = 10;
  const size_t NUM_QUERIES = 200;
  OriginalTable original_table;
  std::vector<std::pair<int64_t, int64_t>> queries;
  IndexType type;
  std::unique_ptr<Index> index;
  UpdateType update_type;

  void run();
  int64_t execute_query(size_t i);
  int64_t execute_query_fs(size_t i);
  void print_result();

private:
  //! Constants for the experiment
  const double SELECTIVITY = 0.01;
  const double DELTA = 0.1;
  const size_t START_UPDATES_AFTER = 50;
  //! Tracks the current update in this execution
  size_t current_update = 0;
  std::vector<double> time;

  [[nodiscard]] std::unique_ptr<Index> get_algorithm();
};