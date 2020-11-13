#include "cracking/avl_tree.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <stdio.h>
#include<memory>

int64_t insertCount = 0;
using namespace std;

int64_t AVLTree::FindLT(int64_t X, AVLNode* T) {
    if (T == nullptr)
        return -1;
    if (X < T->value)
        return FindLT(X, T->left.get());
    else if (X > T->value)
        return FindLT(X, T->right.get());
    else
        return T->offset;
}

int64_t AVLTree::FindLTE(int64_t X, AVLNode* T, int64_t limit) {
    if (T) {
        if (X < T->value)
            if (!T->left) {
                return 0;
            } else {
                return FindLTE(X, T->left.get(), limit);
            }
        else if (X > T->value) {
            if (!T->right) {
                return T->offset;
            } else {
                return FindLTE(X, T->right.get(), limit);
            }
        } else
            return T->offset;
    }

    return limit;
}

AVLNode* AVLTree::FindMin(AVLNode* T) {
    if (T == nullptr) {
        return nullptr;

    } else if (T->left == nullptr) {
        return T;

    } else {
        return FindMin(T->left.get());
    }
}

int64_t AVLNode::Height(AVLNode* P) {
    if (P == nullptr)
        return -1;
    else
        return P->height;
}

//! This function can be called only if K2 has a left child
//! Perform a rotate between a node (K2) and its left child
//! Update heights, then return new root
unique_ptr<AVLNode> AVLTree::SingleRotateWithLeft(unique_ptr<AVLNode> K2) {
    auto K1 = move(K2->left);
    K2->left = move(K1->right);
    K1->right = move(K2);
    K1->right->height = max(AVLNode::Height(K1->right->left.get()), AVLNode::Height(K1->right->right.get())) + 1;
    K1->height = max(AVLNode::Height(K1->left.get()), (int64_t)K1->right->height) + 1;
    //! New root
    return K1;
}

//! This function can be called only if K1 has a right child
//! Perform a rotate between a node (K1) and its right child
//! Update heights, then return new root
std::unique_ptr<AVLNode> AVLTree::SingleRotateWithRight(std::unique_ptr<AVLNode> K1) {
    auto K2 = move(K1->right);
    K1->right = move(K2->left);
    K2->left = move(K1);

    K2->left->height = max(AVLNode::Height(K2->left->left.get()), AVLNode::Height(K2->left->right.get())) + 1;
    K2->height = max(AVLNode::Height(K2->right.get()), (int64_t)K2->left->height) + 1;

    //! New root
    return K2;
}

//! This function can be called only if K3 has a left
//! child and K3's left child has a right child
//! Do the left-right double rotation
//! Update heights, then return new root
std::unique_ptr<AVLNode> AVLTree::DoubleRotateWithLeft(std::unique_ptr<AVLNode> K3) {
    //! Rotate between K1 and K2
    K3->left = SingleRotateWithRight(move(K3->left));
    //! Rotate between K3 and K2
    return SingleRotateWithLeft(move(K3));
}

//! This function can be called only if K1 has a right
//! child and K1's right child has a left child
//! Do the right-left double rotation
//! Update heights, then return new root
std::unique_ptr<AVLNode> AVLTree::DoubleRotateWithRight(std::unique_ptr<AVLNode> K1) {
    //! Rotate between K3 and K2
    K1->right = SingleRotateWithLeft(move(K1->right));

    //! Rotate between K1 and K2
    return SingleRotateWithRight(move(K1));
}

int64_t lookup(int64_t X, AVLNode* T) {
    while (true) {
        if (X == T->value) {
            return T->offset;
        } else if (X < T->value) {
            T = T->left.get();

        } else {
            T = T->right.get();
        }
    }
}

void AVLTree::insert(int64_t value, size_t offset) { root = insert(value, offset, move(root)); }

std::unique_ptr<AVLNode> AVLTree::insert(int64_t value, size_t offset, unique_ptr<AVLNode> node) {
    //! Tree is empty need to create node node
    if (!node) {
        insertCount++;
        node = make_unique<AVLNode>(value, offset);
    } else if (value < node->value) {
        node->left = insert(value, offset, move(node->left));
        if (node->right) {
        }
        if (AVLNode::Height(node->left.get()) - AVLNode::Height(node->right.get()) == 2) {
            if (value < node->left->value) {
                node = SingleRotateWithLeft(move(node));

            } else {
                node = DoubleRotateWithLeft(move(node));
            }
        }
    } else if (value > node->value) {
        node->right = insert(value, offset, move(node->right));
        if (AVLNode::Height(node->right.get()) - AVLNode::Height(node->left.get()) == 2) {
            if (value > node->right->value)
                node = SingleRotateWithRight(move(node));
            else
                node = DoubleRotateWithRight(move(node));
        }
    }
    //! Else pivot is in the tree already; we'll do nothing

    node->height = max(AVLNode::Height(node->left.get()), AVLNode::Height(node->right.get())) + 1;
    return node;
}

unique_ptr<piece> createOffsetPair(AVLNode* first, AVLNode* second, int64_t limit) {
    unique_ptr<piece> op = make_unique<piece>();
    if (first && second) {
        if (first->offset == 0) {
            op->minOffset = first->offset;
            op->minValue = std::numeric_limits<int64_t>::min();
        } else {
            op->minOffset = first->offset + 1;
            op->minValue = first->value;
        }
        op->maxOffset = second->offset;
        op->maxValue = second->value;
        op->right = second;

    } else if (first) {
        if (first->offset == 0) {
            op->minOffset = first->offset;
            op->minValue = std::numeric_limits<int64_t>::min();
        } else {
            op->minOffset = first->offset + 1;
            op->minValue = first->value;
        }
        op->maxOffset = limit;
        op->maxValue = std::numeric_limits<int64_t>::max();
    } else if (second) {
        op->minOffset = 0;
        op->minValue = std::numeric_limits<int64_t>::min();
        op->maxOffset = second->offset;
        op->maxValue = second->value;
        op->right = second;
    } else {
        op->minOffset = 0;
        op->minValue = std::numeric_limits<int64_t>::min();
        op->maxOffset = limit;
        op->maxValue = std::numeric_limits<int64_t>::max();
    }
    return move(op);
}

AVLNode* AVLTree::FindMax(AVLNode* T) {
    if (T != nullptr) {
        while (T->right != nullptr) {
            T = T->right.get();
        }
    }
    return T;
}

unique_ptr<piece> AVLTree::FindNeighborsLT(int64_t X, int64_t limit) {
    AVLNode* first = nullptr;
    AVLNode* second = nullptr;
    auto T = root.get();
    while (T != nullptr) {
        if (X < T->value) {
            second = T;
            T = T->left.get();
        } else if (X > T->value) {
            first = T;
            T = T->right.get();
        } else {
            second = T;
            if (T->left != nullptr) {
                first = FindMax(T->left.get());
            }
            break;
        }
    }

    return move(createOffsetPair(first, second, limit));
}
AVLNode* AVLTree::FindNodeLT(int64_t X) {
    AVLNode* node = nullptr;
    auto T = root.get();
    while (T != nullptr) {
        if (X < T->value) {
            node = T;
            T = T->left.get();
        } else if (X > T->value) {
            T = T->right.get();
        } else {
            node = T;
            break;
        }
    }
    return node;
}

unique_ptr<piece> AVLTree::FindNeighborsGTE(int64_t X, int64_t limit) {
    AVLNode* first = nullptr;
    AVLNode* second = nullptr;
    auto T = root.get();
    while (T != nullptr) {
        if (X < T->value) {
            second = T;
            T = T->left.get();
        } else if (X > T->value) {
            first = T;
            T = T->right.get();
        } else {
            //! this is the only difference from FindNeighborsLT !
            first = T;
            break;
        }
    }

    return move(createOffsetPair(first, second, limit));
}

void _GetNodesInOrder(AVLNode* T, std::vector<AVLNode*>& vector) {
    if (T->left) {
        _GetNodesInOrder(T->left.get(), vector);
    }
    vector.push_back(T);
    if (T->right) {
        _GetNodesInOrder(T->right.get(), vector);
    }
}

vector<AVLNode*> AVLTree::GetNodesInOrder() {
    std::vector<AVLNode*> nodesOrder;
    _GetNodesInOrder(root.get(), nodesOrder);
    return nodesOrder;
}

void Print(AVLNode* T) {

    if (T == nullptr)
        return;

    printf("(%lld,%lld) ", (long long int)T->value, (long long int)T->offset);
    Print(T->right.get());
    Print(T->left.get());
    printf("\n");
}

AVLNode* AVLTree::inOrderSucessor(AVLNode* node) {
    //! if we have a right node we just need to get the min of that node
    if (node->right != nullptr) {
        return FindMin(node->right.get());
    }
    AVLNode* succ;
    auto currentNode = root.get();
    //! Start from root and search for the sucessor down the tree
    while (currentNode) {
        if (node->value < currentNode->value) {
            succ = currentNode;
            currentNode = currentNode->left.get();
        } else if (node->value > currentNode->value) {
            currentNode = currentNode->right.get();
        } else {
            break;
        }
    }
    return succ;
}

AVLNode* AVLTree::inOrderPredecessor(AVLNode* node) {
    //! if we have a left node we just need to get the max of that node
    if (node->left != nullptr) {
        return FindMax(node->left.get());
    }
    AVLNode* succ;
    auto currentNode = root.get();
    //! Start from root and search for the predecessor down the tree
    while (currentNode) {
        if (currentNode->value > node->value) {
            currentNode = currentNode->left.get();
        } else if (currentNode->value < node->value) {
            succ = currentNode;
            currentNode = currentNode->right.get();
        } else {
            break;
        }
    }
    return succ;
}

 void AVLTree::verify_tree(Column &column) {

  auto allNodes = GetNodesInOrder();
  for (size_t i = 0; i < allNodes.size(); i++) {
    if (i == 0) {
      for (size_t j = 0; j <= allNodes[i]->offset; j++)
        if (allNodes[i]->value <= column[j]) {
          //          printTree(column, T, j);
          assert(0);
        }
    } else if (i == allNodes.size() - 1) {
      for (size_t j = allNodes[i]->offset + 1; j < column.size() - 1; j++)
        if (allNodes[i]->value > column[j]) {
          //          printTree(column, T, j);
          assert(0);
        }
    } else {
      for (size_t j = allNodes[i - 1]->offset + 1; j <= allNodes[i]->offset;
           j++) {
        if (allNodes[i]->value <= column[j]) {
          //          printTree(column, T, j);
          assert(0);
        }
        if (allNodes[i - 1]->value > column[j]) {
          //          printTree(column, T, j);
          assert(0);
        }
      }
    }
  }
}
