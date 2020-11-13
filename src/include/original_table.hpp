#include "column.hpp"
class OriginalTable {
public:
  OriginalTable() = default;

  void inititalize(std::vector<int64_t>& data, size_t column_size){
    for (size_t i {}; i < column_size; i++){
      column.push_back(data[i],i);
    }
    for (size_t i {column_size}; i < data.size(); i++){
      updates.push_back(data[i],i);
    }
  }

  int64_t scan(int64_t l_query, int64_t r_query, size_t updates_executed){
    int64_t result {};
    for (size_t i {}; i < column.size(); i++){
      if (column[i] >= l_query && column[i] < r_query){
        result += column[i];
      }
    }
    for (size_t i {}; i < updates_executed; i++){
      if (updates[i] >= l_query && updates[i] < r_query){
        result += updates[i];
      }
    }
    return result;
  }

  Column column;
  Column updates;
};