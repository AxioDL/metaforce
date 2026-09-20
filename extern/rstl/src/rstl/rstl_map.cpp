#include "rstl/red_black_tree.hpp"

namespace {

struct fake_node {
  fake_node* mLeft;
  fake_node* mRight;
  fake_node* mParent;
  rstl::node_color mColor;

  fake_node* get_left() const { return mLeft; }

  void set_left(fake_node* value) { mLeft = value; }

  fake_node* get_right() const { return mRight; }

  void set_right(fake_node* value) { mRight = value; }

  fake_node* get_parent() const { return mParent; }

  void set_parent(fake_node* value) { mParent = value; }

  rstl::node_color get_color() const { return mColor; }

  void set_color(rstl::node_color value) { mColor = value; }
};

struct fake_header {
  fake_node* mLeftmost;
  fake_node* mRightmost;
  fake_node* mRootNode;

  fake_node* get_leftmost() const { return mLeftmost; }

  void set_leftmost(fake_node* value) { mLeftmost = value; }

  fake_node* get_rightmost() const { return mRightmost; }

  void set_rightmost(fake_node* value) { mRightmost = value; }

  fake_node* get_root() const { return mRootNode; }

  void set_root(fake_node* value) { mRootNode = value; }
};

} // namespace

namespace rstl {

void rbtree_rotate_left(void* header_void, void* node_void) {
  fake_header* header = static_cast< fake_header* >(header_void);
  fake_node* node = static_cast< fake_node* >(node_void);

  fake_node* parent = node->mParent;
  fake_node* right = node->mRight;
  fake_node* l = right->mLeft;

  if (parent == nullptr) {
    header->mRootNode = right;
    right->mParent = nullptr;
  } else {
    if (parent->mLeft == node) {
      parent->mLeft = right;
    } else {
      parent->mRight = right;
    }
    right->mParent = node->mParent;
  }
  node->mParent = right;
  right->mLeft = node;
  if (l) {
    l->mParent = node;
  }
  node->mRight = l;
}

void rbtree_rotate_right(void* header_void, void* node_void) {
  fake_header* header = static_cast< fake_header* >(header_void);
  fake_node* node = static_cast< fake_node* >(node_void);

  fake_node* parent = node->mParent;
  fake_node* left = node->mLeft;
  fake_node* r = left->mRight;

  if (parent == nullptr) {
    header->mRootNode = left;
    left->mParent = nullptr;
  } else {
    if (node == parent->mLeft) {
      parent->mLeft = left;
    } else {
      parent->mRight = left;
    }
    left->mParent = node->mParent;
  }
  node->mParent = left;
  left->mRight = node;
  if (r) {
    r->mParent = node;
  }
  node->mLeft = r;
}

void rbtree_rebalance(void* header_void, void* node_void) {
  fake_node* node = static_cast< fake_node* >(node_void);
  fake_header* header = static_cast< fake_header* >(header_void);

  while (node->mParent != nullptr && node->mParent->mColor == kNC_Red) {
    fake_node* p = node->mParent->mParent->mLeft;
    if (node->mParent == p) {
      p = node->mParent->mParent->mRight;
      if ((p != nullptr && p->mColor == kNC_Red)) {
        node->mParent->mColor = kNC_Black;
        p->mColor = kNC_Black;
        node->mParent->mParent->mColor = kNC_Red;
        node = node->mParent->mParent;

      } else {
        if (node == node->mParent->mRight) {
          node = node->mParent;
          rbtree_rotate_left(header, node);
        }
        node->mParent->mColor = kNC_Black;
        node->mParent->mParent->mColor = kNC_Red;
        rbtree_rotate_right(header, node->mParent->mParent);
      }
    } else if (p != nullptr && p->mColor == kNC_Red) {
      node->mParent->mColor = kNC_Black;
      p->mColor = kNC_Black;
      node->mParent->mParent->mColor = kNC_Red;
      node = node->mParent->mParent;

    } else {
      if (node == node->mParent->mLeft) {
        node = node->mParent;
        rbtree_rotate_right(header, node);
      }
      node->mParent->mColor = kNC_Black;
      node->mParent->mParent->mColor = kNC_Red;
      rbtree_rotate_left(header, node->mParent->mParent);
    }
  }
  header->mRootNode->mColor = kNC_Black;
}

void* rbtree_rebalance_for_erase(void* header_void, void* node_void) {
  fake_node* node = static_cast< fake_node* >(node_void);
  fake_node* successor = node;
  fake_node* replacement;
  fake_node* parent;
  fake_header* header = static_cast< fake_header* >(header_void);

  if (node->get_left() == nullptr) {
    replacement = node->get_right();
  } else {
    fake_node* tmp = node->get_right();
    if (tmp == nullptr) {
      replacement = node->get_left();
    } else {
      successor = tmp;
      while (successor->get_left() != nullptr) {
        successor = successor->get_left();
      }
      replacement = successor->get_right();
    }
  }

  if (successor != node) {
    node->get_left()->set_parent(successor);
    successor->set_left(node->get_left());

    if (successor != node->get_right()) {
      parent = successor->get_parent();
      if (replacement != nullptr) {
        replacement->set_parent(successor->get_parent());
      }
      successor->get_parent()->set_left(replacement);
      successor->set_right(node->get_right());
      node->get_right()->set_parent(successor);
    } else {
      parent = successor;
    }

    if (header->get_root() == node) {
      header->set_root(successor);
    } else {
      if (node->get_parent()->get_left() == node) {
        node->get_parent()->set_left(successor);
      } else {
        node->get_parent()->set_right(successor);
      }
    }

    successor->set_parent(node->get_parent());
    node_color c = successor->get_color();
    successor->set_color(node->get_color());
    node->set_color(c);
    successor = node;

  } else {
    parent = successor->get_parent();
    if (replacement != nullptr) {
      replacement->set_parent(parent);
    }
    if (header->get_root() == node) {
      header->set_root(replacement);
    } else {
      if (node->get_parent()->get_left() == node) {
        node->get_parent()->set_left(replacement);
      } else {
        node->get_parent()->set_right(replacement);
      }
    }

    if (header->get_leftmost() == node) {
      if (node->get_right() == nullptr) {
        header->set_leftmost(node->get_parent());
      } else {
        if (replacement == nullptr) {
          header->set_leftmost(replacement);
        } else {
          fake_node* newLeftmost = replacement;
          while (newLeftmost->get_left() != nullptr) {
            newLeftmost = newLeftmost->get_left();
          }
          header->set_leftmost(newLeftmost);
        }
      }
    }

    if (header->get_rightmost() == node) {
      if (node->get_left() == nullptr) {
        header->set_rightmost(node->get_parent());
      } else {
        if (replacement == nullptr) {
          header->set_rightmost(replacement);
        } else {
          fake_node* newRightmost = replacement;
          while (newRightmost->get_right() != nullptr) {
            newRightmost = newRightmost->get_right();
          }
          header->set_rightmost(newRightmost);
        }
      }
    }
  }

  if (successor->get_color() != kNC_Red) {
    fake_node* siblingChild;
    fake_node* sibling;

    while (replacement != header->get_root() &&
           (!replacement || replacement->get_color() == kNC_Black)) {
      sibling = parent->get_left();
      if (replacement == sibling) {
        // Replacement is left child, sibling is on the right
        sibling = parent->get_right();
        if (sibling->get_color() == kNC_Red) {
          sibling->set_color(kNC_Black);
          parent->set_color(kNC_Red);
          rbtree_rotate_left(header, parent);
          sibling = parent->get_right();
        }
        siblingChild = sibling->get_left();
        if ((!siblingChild || siblingChild->get_color() == kNC_Black) &&
            (!sibling->get_right() || sibling->get_right()->get_color() == kNC_Black)) {
          sibling->set_color(kNC_Red);
          replacement = parent;
          parent = parent->get_parent();
        } else {
          if ((sibling->get_right() == nullptr) || (sibling->get_right()->get_color() == kNC_Black)) {
            if (siblingChild != nullptr) {
              siblingChild->set_color(kNC_Black);
            }
            sibling->set_color(kNC_Red);
            rbtree_rotate_right(header, sibling);
            sibling = parent->get_right();
          }
          sibling->set_color(parent->get_color());
          parent->set_color(kNC_Black);
          if (sibling->get_right() != nullptr) {
            sibling->get_right()->set_color(kNC_Black);
          }
          rbtree_rotate_left(header, parent);
          break;
        }
      } else {
        // Replacement is right child, sibling is on the left
        if (sibling->get_color() == kNC_Red) {
          sibling->set_color(kNC_Black);
          parent->set_color(kNC_Red);
          rbtree_rotate_right(header, parent);
          sibling = parent->get_left();
        }
        siblingChild = sibling->get_right();
        if ((!siblingChild || siblingChild->get_color() == kNC_Black) &&
            (!sibling->get_left() || sibling->get_left()->get_color() == kNC_Black)) {
          sibling->set_color(kNC_Red);
          replacement = parent;
          parent = parent->get_parent();
        } else {
          if (!sibling->get_left() || sibling->get_left()->get_color() == kNC_Black) {
            if (siblingChild) {
              siblingChild->set_color(kNC_Black);
            }
            sibling->set_color(kNC_Red);
            rbtree_rotate_left(header, sibling);
            sibling = parent->get_left();
          }
          sibling->set_color(parent->get_color());
          parent->set_color(kNC_Black);
          if (sibling->get_left() != nullptr) {
            sibling->get_left()->set_color(kNC_Black);
          }
          rbtree_rotate_right(header, parent);
          break;
        }
      }
    }
    if (replacement != nullptr) {
      replacement->set_color(kNC_Black);
    }
  }

  return successor;
}

void* rbtree_traverse_forward(const void* header_void, void* node_void) {
  const fake_header* header = static_cast< const fake_header* >(header_void);
  fake_node* node = static_cast< fake_node* >(node_void);

  if (node == nullptr) {
    return header->mLeftmost;
  }
  fake_node* right = node->mRight;
  if ((right == nullptr) && (node->mParent == nullptr)) {
    return nullptr;
  }
  if ((right == nullptr) && (node->mParent->mLeft == node)) {
    return node->mParent;
  }

  if (right != nullptr) {
    fake_node* result = right;
    goto enter_middle;
    do {
      result = right;
    enter_middle:
      right = result->mLeft;
    } while (right != nullptr);
    return result;
  }

  fake_node* parent = nullptr;
  goto enter_final;
  do {
    node = parent;
  enter_final:
    parent = node->mParent;
    if (!parent)
      break;
  } while (parent->mRight == node);

  return parent != nullptr ? parent : nullptr;
}

} // namespace rstl
