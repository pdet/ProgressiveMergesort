#pragma once
#include <cstdlib>
#include <memory>
#include <vector>
#include "column.hpp"
class AVLNode {
  public:
    AVLNode(int64_t value, size_t offset) : value(value), offset(offset), left(nullptr), right(nullptr), height(0){};
    AVLNode(int64_t value) : value(value), offset(0), left(nullptr), right(nullptr), height(0){};

    AVLNode() : value(0), offset(0), left(nullptr), right(nullptr), height(0){};
    static int64_t Height(AVLNode* P);
    //! Value of the split (e.g., the query predicate in the case of
    //! cracking)
    int64_t value;
    //! Offset from the original column
    size_t offset;
    //! The children
    std::unique_ptr<AVLNode> left;
    std::unique_ptr<AVLNode> right;
    //! Height of the Node
    size_t height;
};

//! Cracker Piece
struct piece {
    size_t minOffset;
    size_t maxOffset;
    int64_t minValue;
    int64_t maxValue;
    AVLNode* right = nullptr;
};

class AVLTree {
  public:
    AVLTree() : root(nullptr), maxHeight(0){};
    AVLTree(std::unique_ptr<AVLNode> root) : root(std::move(root)), maxHeight(0){};
    //! Root of the tree
    std::unique_ptr<AVLNode> root;
    //! Maximum height of the tree, that is sued to do range queries in the
    //! tree
    size_t maxHeight;
    //! Main insert method
    void insert(int64_t value, size_t offset);
      //! Prints the Tree
      void printTree(Column &column, size_t
      buggy_element);
      //! Verify Tree correctness
      void verify_tree(Column &column);
    //! Find elements
    int64_t FindLT(int64_t X, AVLNode* T);
    int64_t FindLTE(int64_t X, AVLNode* T, int64_t limit);
    AVLNode* FindMin(AVLNode* T);
    AVLNode* FindMax(AVLNode* T);
    std::unique_ptr<piece> FindNeighborsLT(int64_t X, int64_t limit);
    std::unique_ptr<piece> FindNeighborsGTE(int64_t X, int64_t limit);
    std::vector<AVLNode*> GetNodesInOrder();
    AVLNode* inOrderSucessor(AVLNode* node);
    AVLNode* inOrderPredecessor(AVLNode* node);
    AVLNode* FindNodeLT(int64_t X);

  protected:
    //! Recursive insertion method
    std::unique_ptr<AVLNode> insert(int64_t value, size_t offset, std::unique_ptr<AVLNode> node);
    //! Bunch of rotations
    std::unique_ptr<AVLNode> SingleRotateWithLeft(std::unique_ptr<AVLNode> K2);
    std::unique_ptr<AVLNode> SingleRotateWithRight(std::unique_ptr<AVLNode> K1);
    std::unique_ptr<AVLNode> DoubleRotateWithLeft(std::unique_ptr<AVLNode> K3);
    std::unique_ptr<AVLNode> DoubleRotateWithRight(std::unique_ptr<AVLNode> K1);
};
