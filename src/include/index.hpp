#pragma once
#include "column.hpp"
#include <cstdint>
#include <vector>
enum UpdateType{Complete, Gradual, Ripple,Mergesort};

class Index{
public:
  virtual ~Index()= default;
  virtual void build(Column &original_column,std::pair<int64_t ,int64_t> range_query) = 0;
  virtual void search(Column &original_column,
                      std::pair<int64_t, int64_t> range_query,
                      int64_t &result) = 0;
  virtual void push_update(std::pair<int64_t, int64_t> update) = 0;

};