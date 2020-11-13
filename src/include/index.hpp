#pragma once
#include "column.hpp"
#include <cstdint>
#include <vector>
class Index{
public:
  Column index_column;
  virtual ~Index()= default;
  virtual void build(Column &original_column,std::pair<int64_t ,int64_t> range_query) = 0;
  virtual void search(Column &original_column,
                      std::pair<int64_t, int64_t> range_query,
                      int64_t &result) = 0;
  virtual void push_update(std::pair<int64_t, int64_t> update) = 0;

};