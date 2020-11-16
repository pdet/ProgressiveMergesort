#include "cracking/avl_tree.hpp"
#include <column.hpp>
#include <cracking/cracker_index.hpp>
#include <cassert>
#include <algorithm>
using namespace std;

int64_t binary_search(Column &c, int64_t key, int64_t lower, int64_t upper,
                      bool *foundKey) {
  *foundKey = false;
  while (lower <= upper) {
    int middle = (lower + upper) / 2;
    auto middleElement = c[middle];

    if (middleElement < key) {
      lower = middle + 1;
    } else if (middleElement > key) {
      upper = middle - 1;
    } else {
      *foundKey = true;
      return middle;
    }
  }
  return upper;
}

int64_t binary_search_gte(Column &c, int64_t key, int64_t start, int64_t end) {
  bool found = false;
  int pos = binary_search(c, key, start, end, &found);
  if (found) {
    while (--pos >= start && c[pos] == key)
      ;
  }
  ++pos;
  return pos;
}

int64_t crack_in_two(Column &c, int64_t posL, int64_t posH, int64_t med) {
  int64_t x1 = posL, x2 = posH;
  while (x1 <= x2) {
    if (c[x1] < med)
      x1++;
    else {
      while (x2 >= x1 && (c[x2] >= med))
        x2--;
      if (x1 < x2) {
        c.swap(x1, x2);
        x1++;
        x2--;
      }
    }
  }
  if (x1 < x2)
    printf("Not all elements were inspected!");
  x1--;
  if (x1 < 0)
    x1 = 0;
  return x1;
}

//! Merge Column 'column' with Updates 'updates' between 'posL' and 'posH'
void CrackerIndex::merge(int64_t posL, int64_t posH) {
  //! Column size
  size_t columnSize = index_column.size();
  //! Remaining tuples to be merged
  size_t remaining = posH - posL + 1;
  //! current Position in Cracker column
  size_t currentPosColumn = index_column.size();
  //! current Position in Cracker column of remaining updates in wrong
  //! pieces
  size_t currentPosRemUpdates = index_column.size();
  AVLNode *firstNode = tree.FindMin(tree.root.get());
  AVLNode *currentNode = tree.FindMax(tree.root.get());

  //! We first need to check if we have space in our Cracker Column
  if (index_column.size() < columnSize + remaining) {
    index_column.resize(columnSize + remaining);
  }

  for (size_t i = posL; i <= posH; i++) {
    index_column[currentPosColumn++] = append_list[i];
    //! Check if update belongs to this piece, if yes decrement
    //! remaining
    if (append_list[i] >= currentNode->value) {
      remaining--;
    }
  }
  //! Now we go through all nodes and swap elements until there are no
  //! remaining nodes
  while (remaining > 0 && currentNode != firstNode) {
    AVLNode *previousNode = tree.inOrderPredecessor(currentNode);
    //! Amount of elements that belong in this piece
    size_t elementsBelongHere = 0;
    currentPosColumn = currentNode->offset + 1;
    for (size_t i = currentPosColumn; i < currentPosColumn + remaining; i++) {
      assert(index_column[currentPosRemUpdates] < currentNode->value);
      if (index_column[currentPosRemUpdates] >= previousNode->value) {
        elementsBelongHere++;
      }
      index_column.swap(i, currentPosRemUpdates++);
    }
    currentPosRemUpdates = currentNode->offset + 1;
    currentNode->offset += remaining;
    remaining -= elementsBelongHere;
    currentNode = previousNode;
  }
  if (currentNode == firstNode && remaining > 0) {
    currentPosColumn = currentNode->offset + 1;
    for (size_t i = currentPosColumn; i < currentPosColumn + remaining; i++) {
      index_column.swap(i, currentPosRemUpdates++);
    }
    currentNode->offset += remaining;
    remaining = 0;
  }
  assert(remaining == 0);
}

void CrackerIndex::merge_ripple(int64_t posL, int64_t posH,int64_t high) {
     //! Remaining tuples to be merged
    size_t remaining = posH - posL + 1;

    size_t update_pos;
    //! tmp guy
    pair<int64_t,int64_t > tmp;
    AVLNode* firstNode = tree.FindMin(tree.root.get());
    AVLNode* lastNode = tree.FindMax(tree.root.get());
    AVLNode* currentNode = tree.FindNodeLT(high);

    //! if our query uses the last piece, we can't swap so we do a merge
    //! gradual
    if (currentNode == nullptr || currentNode == lastNode) {
        merge(posL, posH);
        //! Erase from updates whatever was merged
        append_list.erase( posL, posH + 1);
        return;
    }
    AVLNode* leftNode = tree.inOrderPredecessor(currentNode);
    AVLNode* rightNode = tree.inOrderSucessor(currentNode);
    int64_t pieceSize = rightNode->offset - currentNode->offset;

    if (currentNode == lastNode) {
        pieceSize = index_column.size() - currentNode->offset;
        //! We need to resize our column
        if (pieceSize < posH - posL + 1) {
            assert(0);
            index_column.resize(index_column.size() + posH - posL + 1);
        }
        pieceSize = index_column.size() - currentNode->offset;
    }
    assert(pieceSize >= 0);
    vector<AVLNode*> offsetsToUpdate;
    offsetsToUpdate.push_back(currentNode);
    update_pos = currentNode->offset + 1;
    while (posL <= posH) {
        while (pieceSize <= 0) {
            currentNode = rightNode;
            rightNode = tree.inOrderSucessor(currentNode);
            offsetsToUpdate.push_back(currentNode);
            pieceSize = rightNode->offset - currentNode->offset;
        }
        //! Check if piece actually belongs here
        //! If its first node then must be here.
        if (!leftNode || currentNode == firstNode) {
            remaining--;
        }
        //! Ow check if its here
        else if (append_list[posL] >= leftNode->value) {
            remaining--;
        }
        tmp = index_column.get(currentNode->offset + 1);
        index_column.set(append_list.get(posL),currentNode->offset + 1);
        append_list.set(tmp,posL++);
//        tmp = column[currentNode->offset + 1];
//        column[currentNode->offset + 1] = updates[posL];
//        updates[posL++] = tmp;
        //! Increment node offset
        for (const auto& offset : offsetsToUpdate) {
            offset->offset++;
        }
        pieceSize--;
    }
    if (remaining > 0) {
        rightNode = offsetsToUpdate[0];
        currentNode = leftNode;
        leftNode = tree.inOrderPredecessor(currentNode);
        pieceSize = rightNode->offset - currentNode->offset;
        offsetsToUpdate.clear();
        offsetsToUpdate.push_back(currentNode);
        //! Now we need to swap the elements to their right pieces
        while (remaining > 0) {
            size_t swaps = remaining;
            size_t whereWeStartedUpdating = currentNode->offset + 1;
            for (size_t i = 0; i < swaps; i++) {
                while (pieceSize <= 0) {
                    currentNode = rightNode;
                    rightNode = tree.inOrderSucessor(currentNode);
                    offsetsToUpdate.push_back(currentNode);
                    pieceSize = rightNode->offset - currentNode->offset;
                }
                //! Check if piece actually belongs here
                //! If its first node then must be here.
                if (!leftNode || currentNode == firstNode) {
                    remaining--;
                }
                //! Ow check if its here
                else if (index_column[update_pos + i] >= leftNode->value) {
                    remaining--;
                }
                index_column.swap(currentNode->offset + 1,update_pos + i);
//                tmp = column[currentNode->offset + 1];
//                column[currentNode->offset + 1] = column[update_pos + i];
//                column[update_pos + i] = tmp;
                //! Increment node offset
                for (const auto& offset : offsetsToUpdate) {
                    offset->offset++;
                }
                pieceSize--;
            }
            //! We are done
            if (remaining == 0) {
                break;
            }
            //! Move to piece in the left
            rightNode = offsetsToUpdate[0];
            currentNode = leftNode;
            leftNode = tree.inOrderPredecessor(currentNode);
            pieceSize = rightNode->offset - currentNode->offset;
            offsetsToUpdate.clear();
            update_pos = whereWeStartedUpdating;
            offsetsToUpdate.push_back(currentNode);
        }
    }
}


void CrackerIndex::build(Column &original_column,
                         pair<int64_t, int64_t> range_query) {
  if (index_column.empty()) {
    //! We need to copy the column
    index_column = original_column;
  }
  //! We need to check if we gotta merge stuff
  if (!append_list.empty()) {
    switch (update_type) {
    case UpdateType::Complete: {
      sort(append_list.data.begin(), append_list.data.end());
      merge(0, append_list.size() - 1);
//      tree.verify_tree(index_column);
      append_list.clear();
      break;
    }
    case UpdateType::Gradual: {
      if (!updates_sorted) {
        sort(append_list.data.begin(), append_list.data.end());
        updates_sorted = true;
      }
      int64_t initial_offset = binary_search_gte(append_list, range_query.first,
                                                 0, append_list.size() - 1);
      int64_t final_offset = binary_search_gte(append_list, range_query.second,
                                               0, append_list.size() - 1);

      if (final_offset > initial_offset) {
        merge(initial_offset, final_offset - 1);
        append_list.erase(initial_offset, final_offset);
      }
      break;
    }
    case UpdateType::Ripple: {
      sort(append_list.data.begin(), append_list.data.end());
        updates_sorted = true;
      int64_t initial_offset = binary_search_gte(append_list, range_query.first,
                                                 0, append_list.size() - 1);
      //! Nothing to merge
      if (initial_offset >= append_list.size()) {
        break;
      }
      assert(append_list[initial_offset] >= range_query.first);
//      while (append_list[initial_offset] < range_query.first) {
//        initial_offset++;
//      }
      int64_t final_offset = binary_search_gte(append_list, range_query.second,
                                               0, append_list.size() - 1);
      //! Nothing to merge
      if (final_offset < 0) {
        break;
      } else if (final_offset >= append_list.size()) {
        final_offset = append_list.size() - 1;
      }
      while (append_list[final_offset] >= range_query.second) {
        final_offset--;
        //! Nothing to merge
        if (final_offset < 0) {
          break;
        }
      }
      if (final_offset >= initial_offset) {
        merge_ripple(initial_offset, final_offset, range_query.second);
      }
      break;
    }
    default:
      throw "Not Implemented";
    }
  }
  //! Partitioning Column and Inserting in Cracker Indexing
  std::unique_ptr<piece> p1;
  std::unique_ptr<piece> p2;

  p1 = tree.FindNeighborsLT(range_query.first, index_column.size() - 1);
  int64_t offset = crack_in_two(index_column, p1->minOffset, p1->maxOffset,
                                range_query.first);
  tree.insert(range_query.first, offset);
  p1 = tree.FindNeighborsLT(range_query.second, index_column.size() - 1);
  offset = crack_in_two(index_column, p1->minOffset, p1->maxOffset,
                        range_query.second);
  tree.insert(range_query.second, offset);
}

void CrackerIndex::search(Column &original_column,
                          std::pair<int64_t, int64_t> range_query,
                          int64_t &result) {
  std::unique_ptr<piece> p1;
  std::unique_ptr<piece> p2;
  p1 = tree.FindNeighborsGTE(range_query.first, index_column.size() - 1);
  p2 = tree.FindNeighborsLT(range_query.second, index_column.size() - 1);
  result = index_column.scan(p1->minOffset, p2->maxOffset);
}

void CrackerIndex::push_update(std::pair<int64_t, int64_t> update) {
  updates_sorted = false;
  append_list.push_back(update.first, update.second);
}