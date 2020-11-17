#include "experiment.hpp"
#include <chrono>
#include <iostream>
#include <random>
#include <algorithm>
#include "progressive/progressive_index.hpp"

using namespace std;
using namespace chrono;

Experiment::Experiment(size_t column_size, size_t num_updates, size_t frequency, IndexType index_type, UpdateType up_type)
    :COLUMN_SIZE(column_size),NUM_UPDATES(num_updates),FREQUENCY(frequency), type(index_type),update_type(up_type){
  //! Generate Column and Updates
  size_t update_size =
      ((NUM_QUERIES - START_UPDATES_AFTER) / FREQUENCY) * NUM_UPDATES;
  {
    vector<int64_t> all_data;
    for (size_t i{}; i < COLUMN_SIZE + update_size; i++) {
      all_data.push_back(i);
    }
    std::mt19937 g(3);
    shuffle(all_data.begin(), all_data.end(), g);
    original_table.inititalize(all_data, COLUMN_SIZE);
  }

  //! Generate Workload
  for (size_t i{}; i < NUM_QUERIES; i++) {
    int64_t low_predicate =
        rand() % COLUMN_SIZE + update_size * (1 - SELECTIVITY);
    int64_t high_predicate =
        low_predicate + (COLUMN_SIZE + update_size) * SELECTIVITY;
    queries.emplace_back(low_predicate, high_predicate);
  }
  //! Set Index Structure
  index = get_algorithm();

}
Experiment::Experiment(size_t num_queries, size_t column_size, IndexType index_type, UpdateType up_type, size_t num_updates,
           size_t frequency, size_t start_updates_after, double delta):NUM_QUERIES(num_queries),
           START_UPDATES_AFTER(start_updates_after),DELTA(delta),COLUMN_SIZE(column_size),NUM_UPDATES(num_updates),
           FREQUENCY(frequency), type(index_type),update_type(up_type){
     //! Generate Column and Updates
  size_t update_size =
      ((NUM_QUERIES - START_UPDATES_AFTER) / FREQUENCY) * NUM_UPDATES;
  {
    vector<int64_t> all_data;
    for (size_t i{}; i < COLUMN_SIZE + update_size; i++) {
      all_data.push_back(i);
    }
    std::mt19937 g(3);
    shuffle(all_data.begin(), all_data.end(), g);
    original_table.inititalize(all_data, COLUMN_SIZE);
  }

  //! Generate Workload
  for (size_t i{}; i < NUM_QUERIES; i++) {
    int64_t low_predicate =
        rand() % COLUMN_SIZE + update_size * (1 - SELECTIVITY);
    int64_t high_predicate =
        low_predicate + (COLUMN_SIZE + update_size) * SELECTIVITY;
    queries.emplace_back(low_predicate, high_predicate);
  }
  //! Set Index Structure
  index = get_algorithm();
}


void Experiment::run() {
  for (size_t i {}; i < queries.size(); i ++){
    auto start_timer = system_clock::now();
    execute_query(i);
    std::cout << duration<double>(system_clock::now() - start_timer).count() << std::endl;
  }
}

int64_t Experiment::execute_query(size_t i){
  //! check if we need to do updates
  if (i > START_UPDATES_AFTER && i%FREQUENCY == 0){
    for (size_t j {}; j < NUM_UPDATES; j++){
      index->push_update(original_table.updates.data[current_update++]);
    }
  }
  //! Now execute the query
  int64_t result;
  index->build(original_table.column, queries[i]);
  index->search(original_table.column, queries[i], result);
  return result;
}

int64_t Experiment::execute_query_fs(size_t i){
  return original_table.scan(queries[i].first,queries[i].second,current_update);
}

void Experiment::print_result(){
    //! Print results
  for (auto const &t : time) {
    std::cout << t << std::endl;
  }
}

std::unique_ptr<Index> Experiment::get_algorithm() {
  switch (type) {
  case IndexType::Cracking:
    return std::make_unique<CrackerIndex>(update_type);
    case IndexType::Progressive:
    return std::make_unique<ProgressiveIndex>(original_table.column,DELTA,update_type);
  default:
    throw "Not implemented";
  }
}