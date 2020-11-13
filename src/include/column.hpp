#pragma once
#include <vector>


class Column {
public:
  Column()=default;
  ~Column()= default;
    //! Copy Constructor
    Column (const Column &original)
    {
        for (auto const& o_data: original.data){
          data.push_back(o_data);
        }
    }

    void swap(size_t i, size_t j){
      std::swap(data[i],data[j]);
    }
    void clear(){
      data.clear();
    }
    void push_back(int64_t o_data, int64_t o_row_id){
      data.emplace_back(o_data,o_row_id);
    }

    int64_t & operator[](int index) {
      return data[index].first;
    }

    [[nodiscard]] bool empty() const{
      return data.empty();
    }
    [[nodiscard]] size_t size() const{
      return data.size();
    }

    void resize(size_t new_size){
      data.resize(new_size);
    }

    std::pair<int64_t ,int64_t > get(size_t i){
      return data[i];
    }

    void set(std::pair<int64_t ,int64_t > value, size_t i){
      data[i] = value;
    }

    void erase(size_t i_of, size_t e_of){
     data.erase(data.begin()+i_of,data.begin()+e_of);
    }

    int64_t scan(size_t low_of, size_t high_of){
      int64_t result{};
      for (;low_of<=high_of;low_of++){
        result += data[low_of].first;
      }
      return result;
    }
	std::vector<std::pair<int64_t,size_t>> data;
};

