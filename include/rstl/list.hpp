#ifndef _RSTL_LIST
#define _RSTL_LIST

#include "types.h"

#include "rstl/construct.hpp"
#include "rstl/functional.hpp"
#include "rstl/iterator.hpp"
#include "rstl/allocator.hpp"
namespace rstl {
template < typename T, typename Alloc = rmemory_allocator >
class list {
public:
  class iterator;
  class const_iterator;

  // private:
#pragma pack(push, 1)
  struct node;
#pragma pack(pop)

public:
  list(const Alloc& alloc = Alloc())
  : x0_allocator(alloc)
  , x4_start(reinterpret_cast< node* >(&xc_empty_prev))
  , x8_end(reinterpret_cast< node* >(&xc_empty_prev))
  , xc_empty_prev(reinterpret_cast< node* >(&xc_empty_prev))
  , x10_empty_next(reinterpret_cast< node* >(&xc_empty_prev))
  , x14_count(0) {}

  struct destroy_helper {
    destroy_helper(list* l) : x0_list(l), x4_active(true) {}
    ~destroy_helper() {
      if (x4_active) {
        x0_list->destroy();
      }
    }
    void release() { x4_active = false; }

  private:
    list* x0_list;
    bool x4_active;
  };

  list(const list& other)
  : x0_allocator(other.x0_allocator)
  , x4_start(reinterpret_cast< node* >(&xc_empty_prev))
  , x8_end(reinterpret_cast< node* >(&xc_empty_prev))
  , xc_empty_prev(reinterpret_cast< node* >(&xc_empty_prev))
  , x10_empty_next(reinterpret_cast< node* >(&xc_empty_prev))
  , x14_count(0) {
    destroy_helper dh(this);
    insert(end(), other.begin(), other.end());
    dh.release();
  }

  list& operator=(const list& other) {
    if (this == &other) {
      return *this;
    }
    clear();
    insert(end(), other.begin(), other.end());
    return *this;
  }

  ~list();
  node* do_erase(node* item);

  void push_front(const T& val) { do_insert_before(x4_start, val); }
  void push_back(const T& val) { do_insert_before(x8_end, val); }
  void clear() { erase(begin(), end()); }

  int size() const { return x14_count; }
  bool empty() const { return x14_count == 0; }

  T& front() { return *x4_start->get_value(); }
  T& back() { return *x8_end->get_prev()->get_value(); }
  const T& front() const { return *x4_start->get_value(); }

  void pop_front() { erase(x4_start); }

  iterator begin() { return iterator(x4_start); }
  const_iterator begin() const { return const_iterator(x4_start); }
  iterator end() { return iterator(x8_end); }
  const_iterator end() const { return const_iterator(x8_end); }

  iterator erase(const iterator& item) { return do_erase(item.get_node()); }
  iterator erase(const iterator& start, const iterator& end) {
    iterator it = start;
    while (it != end) {
      it = erase(it);
    }
    return it;
  }

  struct node {
    node* x0_prev;
    node* x4_next;
    ALIGNAS(T) uchar x8_item[sizeof(T)];

    node(node* prev, node* next) : x0_prev(prev), x4_next(next) {}

    node* get_prev() const { return x0_prev; }
    node* get_next() const { return x4_next; }
    void set_prev(node* prev) { x0_prev = prev; }
    void set_next(node* next) { x4_next = next; }
    T* get_value() { return reinterpret_cast< T* >(&x8_item); }
    const T* get_value() const { return reinterpret_cast< const T* >(&x8_item); }
  };

  node* create_node(node* prev, node* next, const T& val) {
    node* n;
    x0_allocator.allocate(n, 1);
    new (n) node(prev, next);
    new (n->x8_item) T(val);
    return n;
  }

  node* do_insert_before(node* n, const T& val) {
    node* const nn = create_node(n->x0_prev, n, val);
    if (n == x4_start) {
      x4_start = nn;
    }
    nn->get_prev()->set_next(nn);
    nn->get_next()->set_prev(nn);
    ++x14_count;

    return nn;
  }

  iterator insert(const iterator& pos, const T& val);

  template < typename InputIterator >
  void insert(const iterator& pos, InputIterator first, InputIterator last);

  void destroy() {
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    clear();
#else
    iterator last = end();
    iterator first = begin();
    erase(first, last);
#endif
  }

  void remove(const T& val);

  template < typename Pred >
  void remove_if(Pred pred) {
    node* it = x4_start;
    while (it != x8_end) {
      if (pred(*it->get_value())) {
        it = do_erase(it);
      } else {
        it = it->get_next();
      }
    }
  }

  template < typename Cmp >
  void sort(Cmp cmp) {
    iterator it = begin();
    while (it != end()) {
      iterator min = it;
      iterator next = it;
      ++next;
      for (; next != end(); ++next) {
        if (cmp(*min, *next)) {
          min = next;
        }
      }

      next = it;
      ++next;
      exchange(it.get_node(), min.get_node());
      it = next;
    }
  }

  void exchange(node* nodeA, node* nodeB);

public:
  class const_iterator {
  public:
    typedef bidirectional_iterator_tag iterator_category;
    typedef int difference_type;
    typedef T* value_type;

    const_iterator() : current(nullptr) {}
    const_iterator(node* const begin) : current(begin) {}
    const_iterator& operator++() {
      this->current = this->current->x4_next;
      return *this;
    }
    const_iterator operator++(int) const { return const_iterator(this->current->x4_next); }
    const_iterator& operator--() {
      this->current = this->current->x0_prev;
      return *this;
    }
    const_iterator operator--(int) const { return const_iterator(this->current->x0_prev); }
    const T* get_pointer() const { return current->get_value(); }
    const T& operator*() const { return *current->get_value(); }
    const T* operator->() const { return current->get_value(); }
    bool operator==(const const_iterator& other) const { return current == other.current; }
    bool operator!=(const const_iterator& other) const { return current != other.current; }

    node* get_node() const { return current; }

  protected:
    node* current;
  };

  class iterator : public const_iterator {
  public:
    typedef T* value_type;

    iterator() : const_iterator(nullptr) {}
    iterator(node* const begin) : const_iterator(begin) {}
    iterator& operator++() {
      this->current = this->current->x4_next;
      return *this;
    }
    iterator operator++(int) {
      node* cur = this->current;
      this->current = this->current->x4_next;
      return cur;
    }
    iterator& operator--() {
      this->current = this->current->x0_prev;
      return *this;
    }
    iterator operator--(int) { return iterator(this->curent->x0_prev); }
    T* get_pointer() const { return this->current->get_value(); }
    T& operator*() const { return *this->current->get_value(); }
    T* operator->() const { return this->current->get_value(); }
    bool operator==(const iterator& other) const { return this->current == other.current; }
    bool operator!=(const iterator& other) const { return this->current != other.current; }
  };

private:
  Alloc x0_allocator;
  node* x4_start;
  node* x8_end;
  ALIGNAS(node) node* xc_empty_prev;
  node* x10_empty_next;
  int x14_count;
};

template < typename T, typename Alloc >
inline typename list< T, Alloc >::iterator list< T, Alloc >::insert(const iterator& pos, const T& val) {
  node* const result = do_insert_before(pos.get_node(), val);
  return iterator(result);
}

template < typename T, typename Alloc >
template < typename InputIterator >
inline void list< T, Alloc >::insert(const iterator& pos, InputIterator first, InputIterator last) {
  for (InputIterator it = first; it != last; ++it) {
    insert(pos, *it);
  }
}

template < typename T, typename Alloc >
void list< T, Alloc >::remove(const T& val) {
  rstl::equal_to< T > equal;
  remove_if(rstl::bind1st(equal, val));
}

template < typename T, typename Alloc >
list< T, Alloc >::~list() {
  node* cur = x4_start;
  while (cur != x8_end) {
    node* it = cur;
    node* next = cur->get_next();
    cur = next;
    it->get_value()->~T();
    x0_allocator.deallocate(it);
  }
}

template < typename T, typename Alloc >
typename list< T, Alloc >::node* list< T, Alloc >::do_erase(node* node) {
  typename list< T, Alloc >::node* result = node->get_next();
  if (node == x4_start) {
    x4_start = node->get_next();
  }
  node->get_prev()->set_next(node->get_next());
  node->get_next()->set_prev(node->get_prev());
  node->get_value()->~T();
  x0_allocator.deallocate(node);
  x14_count--;
  return result;
}

template < typename T, typename Alloc >
void list< T, Alloc >::exchange(node* nodeA, node* nodeB) {
  if (nodeA == nodeB) {
    return;
  }

  if (nodeA == this->x4_start) {
    this->x4_start = nodeB;
  } else if (nodeB == this->x4_start) {
    this->x4_start = nodeA;
  }

  node* nodeANext = nodeA->x4_next;
  node* nodeAPrev = nodeA->x0_prev;
  node* nodeBNext = nodeB->x4_next;
  node* nodeBPrev = nodeB->x0_prev;

  if (nodeBNext != nodeA) {
    nodeBNext->x0_prev = nodeA;
    nodeA->x4_next = nodeBNext;
    nodeAPrev->x4_next = nodeB;
    nodeB->x0_prev = nodeAPrev;
  } else {
    nodeA->x4_next = nodeB;
    nodeB->x0_prev = nodeA;
  }

  if (nodeBPrev != nodeA) {
    nodeBPrev->x4_next = nodeA;
    nodeA->x0_prev = nodeBPrev;
    nodeANext->x0_prev = nodeB;
    nodeB->x4_next = nodeANext;
  } else {
    nodeB->x4_next = nodeA;
    nodeA->x0_prev = nodeB;
  }
}

} // namespace rstl

#endif // _RSTL_LIST
