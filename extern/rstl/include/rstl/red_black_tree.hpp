#ifndef _RSTL_RED_BLACK_TREE
#define _RSTL_RED_BLACK_TREE

#include "types.h"

#include "rstl/functional.hpp"
#include "rstl/iterator.hpp"
#include "rstl/pair.hpp"
#include "rstl/allocator.hpp"
namespace rstl {

enum node_color {
  kNC_Black,
  kNC_Red,
};

void rbtree_rebalance(void*, void*);
void* rbtree_traverse_forward(const void*, void*);
void* rbtree_rebalance_for_erase(void* header_void, void* node_void);

template < typename T, typename P, bool IsMulti, typename S = select1st< P >, typename Cmp = less< T >,
           typename Alloc = rmemory_allocator >
class red_black_tree {
private:
  struct node {
    node* mLeft;
    node* mRight;
    node* mParent;
    node_color mColor;
    ALIGNAS(P) uchar mValue[sizeof(P)];

    node(node* left, node* right, node* parent, node_color color, const P& value)
    : mLeft(left), mRight(right), mParent(parent), mColor(color) {
      new (mValue) P(value);
    }
    ~node() { reinterpret_cast< P* >(mValue)->~P(); }

    P* get_value() { return reinterpret_cast< P* >(&mValue); }
    const P* get_value() const { return reinterpret_cast< const P* >(&mValue); }

    node* get_left() { return mLeft; }
    void set_left(node* n) { mLeft = n; }
    node* get_right() { return mRight; }
    void set_right(node* n) { mRight = n; }
    node_color get_color() const { return mColor; }
    void set_parent(node* n) { mParent = n; }
  };
  class header {
  public:
    header() : mLeftmost(nullptr), mRightmost(nullptr), mRootNode(nullptr) {}
    void set_root(node* n) { mRootNode = n; }
    void set_leftmost(node* n) { mLeftmost = n; }
    void set_rightmost(node* n) { mRightmost = n; }

    node* get_root() const { return mRootNode; }
    node* get_leftmost() const { return mLeftmost; }
    node* get_rightmost() const { return mRightmost; }

  private:
    node* mLeftmost;
    node* mRightmost;
    node* mRootNode;
  };

public:
  struct const_iterator {
    typedef int difference_type;
    typedef forward_iterator_tag iterator_category;
    typedef P value_type;

    node* mNode;
    const header* mHeader;
    const_iterator(node* node, const header* header)
    : mNode(node), mHeader(header) {}

    const P* operator->() const { return mNode->get_value(); }
    const P& operator*() const { return *mNode->get_value(); }
    bool operator==(const const_iterator& other) const {
      return mNode == other.mNode && mHeader == other.mHeader;
    }
    bool operator!=(const const_iterator& other) const {
      return (mNode != other.mNode || mHeader != other.mHeader);
    }

    const_iterator& operator++() {
      mNode = static_cast< node* >(rbtree_traverse_forward(static_cast< const void* >(mHeader),
                                                           static_cast< void* >(mNode)));
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator result = *this;
      mNode = static_cast< node* >(rbtree_traverse_forward(static_cast< const void* >(mHeader),
                                                           static_cast< void* >(mNode)));
      return result;
    }
  };
  struct iterator : public const_iterator {
    iterator(node* node, const header* header) : const_iterator(node, header) {}

    P* operator->() const { return const_iterator::mNode->get_value(); }
    P& operator*() const { return *const_iterator::mNode->get_value(); }
    node* get_node() { return const_iterator::mNode; }

    iterator& operator++() {
      const_iterator::mNode = static_cast< node* >(
          rbtree_traverse_forward(static_cast< const void* >(const_iterator::mHeader),
                                  static_cast< void* >(const_iterator::mNode)));
      return *this;
    }

    iterator operator++(int) {
      iterator result = *this;
      ++*this;
      return result;
    }
  };

  red_black_tree(const S& selector = S(), const Cmp& cmp = Cmp(), const Alloc& alloc = Alloc())
  : x0_selector(selector), x1_cmp(cmp), x2_allocator(alloc), x4_count(0) {}
  red_black_tree(CInputStream& in, const S& selector = S(), const Cmp& cmp = Cmp(),
                 const Alloc& alloc = Alloc());
  red_black_tree(const red_black_tree& other)
  : x0_selector(other.x0_selector)
  , x1_cmp(other.x1_cmp)
  , x2_allocator(other.x2_allocator)
  , x4_count(other.x4_count) {
    node* root = copy_from(other.x8_header.get_root());
    x8_header.set_leftmost(leftmost(root));
    x8_header.set_rightmost(rightmost(root));
    x8_header.set_root(root);
  }
  ~red_black_tree() { destroy(); }

  pair< iterator, bool > insert_into(node* n, const P& item);
  pair< iterator, bool > insert(const P& item) { return insert_into(x8_header.get_root(), item); }

  const_iterator begin() const {
    // TODO
    return const_iterator(x8_header.get_leftmost(), &x8_header);
  }
  const_iterator end() const {
    // TODO
    return const_iterator(nullptr, &x8_header);
  }

  iterator begin() {
    // TODO
    return iterator(x8_header.get_leftmost(), &x8_header);
  }
  iterator end() {
    // TODO
    return iterator(nullptr, &x8_header);
  }

  const_iterator find(const T& key) const { return const_iterator(find_node(key), &x8_header); }

  iterator find(const T& key) { return iterator(find_node(key), &x8_header); }

  node* find_node(const T& key) const {
    node* n = x8_header.get_root();
    node* needle = nullptr;
    while (n != nullptr) {
      if (!x1_cmp(x0_selector(*n->get_value()), key)) {
        needle = n;
        n = n->get_left();
      } else {
        n = n->get_right();
      }
    }
    bool noResult = false;
    if (needle == nullptr || x1_cmp(key, x0_selector(*needle->get_value()))) {
      noResult = true;
    }
    return noResult ? nullptr : needle;
  }

  iterator lower_bound(const T& key) { return iterator(find_lower_bound(key), &x8_header); }
  const_iterator lower_bound(const T& key) const {
    return const_iterator(find_lower_bound(key), &x8_header);
  }

  iterator upper_bound(const T& key) { return iterator(find_upper_bound(key), &x8_header); }
  const_iterator upper_bound(const T& key) const {
    return const_iterator(find_upper_bound(key), &x8_header);
  }

  pair< iterator, iterator > equal_range(const T& key) {
    return pair< iterator, iterator >(lower_bound(key), upper_bound(key));
  }
  pair< const_iterator, const_iterator > equal_range(const T& key) const {
    return pair< const_iterator, const_iterator >(lower_bound(key), upper_bound(key));
  }

  iterator erase(iterator it) {
    node* n = it.get_node();
    ++it;
    free_node(rebalance_for_erase(n));
    x4_count--;
    return it;
  }

  int erase(const T& key) {
    pair< iterator, iterator > range = equal_range(key);
    int count = rstl::distance(range.first, range.second);
    iterator it = range.first;
    while (it != range.second) {
      erase(it++);
    }
    return count;
  }

  void clear() {
    node* root = x8_header.get_root();
    if (root != nullptr) {
      free_node_and_sub_nodes(root);
    }
    x8_header.set_root(nullptr);
    x8_header.set_leftmost(nullptr);
    x8_header.set_rightmost(nullptr);
    x4_count = 0;
  }

  int size() const { return x4_count; }

private:
  node* find_lower_bound(const T& key) const {
    node* n = x8_header.get_root();
    node* result = nullptr;
    while (n != nullptr) {
      if (!x1_cmp(x0_selector(*n->get_value()), key)) {
        result = n;
        n = n->get_left();
      } else {
        n = n->get_right();
      }
    }
    return result;
  }

  node* find_upper_bound(const T& key) const {
    node* n = x8_header.get_root();
    node* result = nullptr;
    while (n != nullptr) {
      if (x1_cmp(key, x0_selector(*n->get_value()))) {
        result = n;
        n = n->get_left();
      } else {
        n = n->get_right();
      }
    }
    return result;
  }

  S x0_selector;
  Cmp x1_cmp;
  Alloc x2_allocator;
  int x4_count;
  header x8_header;

  node* leftmost(node* n) {
    node* ret;
    if ((ret = n) != nullptr) {
      while (node* next = ret->get_left()) {
        ret = next;
      }
    }
    return ret;
  }

  node* rightmost(node* n) {
    if (n != nullptr) {
      while (n->get_right() != nullptr) {
        n = n->get_right();
      }
    }
    return n;
  }

  void free_node_and_sub_nodes(node* n);
  node* copy_from(node* n);

  node* create_node(node* left, node* right, node* parent, node_color color, const P& value) {
    node* n;
    Alloc::allocate(n, 1);
    new (n) node(left, right, parent, color, value);
    return n;
  }

  void free_node(node* n) {
    n->~node();
    Alloc::deallocate(n);
  }

  void rebalance(node* n) { rbtree_rebalance(&x8_header, n); }

  node* rebalance_for_erase(node* n) {
    return static_cast< node* >(rbtree_rebalance_for_erase(&x8_header, n));
  }

  void destroy() { clear(); }
};

template < typename T, typename P, bool IsMulti, typename S, typename Cmp, typename Alloc >
pair< typename red_black_tree< T, P, IsMulti, S, Cmp, Alloc >::iterator, bool >
red_black_tree< T, P, IsMulti, S, Cmp, Alloc >::insert_into(node* start, const P& item) {
  if (start == nullptr) {
    x8_header.set_root(create_node(nullptr, nullptr, nullptr, kNC_Black, item));
    x4_count += 1;
    x8_header.set_leftmost(x8_header.get_root());
    x8_header.set_rightmost(x8_header.get_root());
    return pair< iterator, bool >(iterator(x8_header.get_root(), &x8_header), true);

  } else {
    node* n = start;
    node* newNode = nullptr;
    while (newNode == nullptr) {
      bool firstComp = x1_cmp(x0_selector(item), x0_selector(*n->get_value()));
      if (!IsMulti && !firstComp && !x1_cmp(x0_selector(*n->get_value()), x0_selector(item))) {
        return pair< iterator, bool >(iterator(n, &x8_header), false);
      }
      if (firstComp) {
        if (n->get_left() == nullptr) {
          newNode = create_node(nullptr, nullptr, n, kNC_Red, item);
          n->set_left(newNode);
          if (n == x8_header.get_leftmost()) {
            x8_header.set_leftmost(newNode);
          }
        } else {
          n = n->get_left();
        }
      } else {
        if (n->get_right() == nullptr) {
          newNode = create_node(nullptr, nullptr, n, kNC_Red, item);
          n->set_right(newNode);
          if (n == x8_header.get_rightmost()) {
            x8_header.set_rightmost(newNode);
          }
        } else {
          n = n->get_right();
        }
      }
    }
    x4_count += 1;
    rebalance(newNode);
    return pair< iterator, bool >(iterator(newNode, &x8_header), true);
  }
}

template < typename T, typename P, bool IsMulti, typename S, typename Cmp, typename Alloc >
void red_black_tree< T, P, IsMulti, S, Cmp, Alloc >::free_node_and_sub_nodes(node* n) {
  if (node* left = n->get_left()) {
    free_node_and_sub_nodes(left);
  }
  if (node* right = n->get_right()) {
    free_node_and_sub_nodes(right);
  }
  free_node(n);
}

template < typename T, typename P, bool IsMulti, typename S, typename Cmp, typename Alloc >
typename red_black_tree< T, P, IsMulti, S, Cmp, Alloc >::node*
red_black_tree< T, P, IsMulti, S, Cmp, Alloc >::copy_from(node* n) {
  if (n == nullptr) {
    return nullptr;
  }
  node* left = nullptr;
  node* right = nullptr;
  if (n->get_left() != nullptr) {
    left = copy_from(n->get_left());
  }
  if (n->get_right() != nullptr) {
    right = copy_from(n->get_right());
  }
  node* const ret = create_node(left, right, nullptr, n->get_color(), *n->get_value());
  if (left != nullptr) {
    left->set_parent(ret);
  }
  if (right != nullptr) {
    right->set_parent(ret);
  }
  return ret;
}

}; // namespace rstl

#endif // _RSTL_RED_BLACK_TREE
