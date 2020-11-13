#pragma once
#include "index.hpp"
#include "cracking/avl_tree.hpp"
enum UpdateType{Complete, Gradual, Ripple};
class CrackerIndex: public Index{
public:
  ~CrackerIndex() override = default;

  explicit CrackerIndex(UpdateType u_type):update_type(u_type){};
  void build(Column &original_column,std::pair<int64_t ,int64_t> range_query) override;
  void search(Column &original_column, std::pair<int64_t, int64_t> range_query,
              int64_t &result) override;
  void push_update(std::pair<int64_t, int64_t> update) override;
private:
  AVLTree tree;
  UpdateType update_type;
  Column append_list;
  bool updates_sorted = false;
  void merge(int64_t posL, int64_t posH);
  void merge_ripple(int64_t posL, int64_t posH,int64_t high);

};