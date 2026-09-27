#ifndef _RSTL_LIST
#define _RSTL_LIST

#include "types.h"

#include "rstl/RstlVersions.h"
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
  : mAllocator(alloc)
  , mStart(reinterpret_cast< node* >(&mEmpty_prev))
  , mEnd(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmpty_prev(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmpty_next(reinterpret_cast< node* >(&mEmpty_prev))
  , mCount(0) {}

  struct destroy_helper {
    destroy_helper(list* l) : mList(l), mActive(true) {}
    ~destroy_helper() {
      if (mActive) {
        mList->destroy();
      }
    }
    void release() { mActive = false; }

  private:
    list* mList;
    bool mActive;
  };

  list(const list& other)
  : mAllocator(other.mAllocator)
  , mStart(reinterpret_cast< node* >(&mEmpty_prev))
  , mEnd(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmpty_prev(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmpty_next(reinterpret_cast< node* >(&mEmpty_prev))
  , mCount(0) {
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

  void push_front(const T& val) { do_insert_before(mStart, val); }
  void push_back(const T& val) { do_insert_before(mEnd, val); }
  void clear() { erase(begin(), end()); }

  int size() const { return mCount; }
  bool empty() const { return mCount == 0; }

  T& front() { return *mStart->get_value(); }
  T& back() { return *mEnd->get_prev()->get_value(); }
  const T& front() const { return *mStart->get_value(); }

  void pop_front() { erase(mStart); }

  iterator begin() { return iterator(mStart); }
  const_iterator begin() const { return const_iterator(mStart); }
  iterator end() { return iterator(mEnd); }
  const_iterator end() const { return const_iterator(mEnd); }

  iterator erase(const iterator& item) { return do_erase(item.get_node()); }
  iterator erase(const iterator& start, const iterator& end) {
    iterator it = start;
    while (it != end) {
      it = erase(it);
    }
    return it;
  }

  struct node {
    node* mPrev;
    node* mNext;
    ALIGNAS(T) uchar mItem[sizeof(T)];

    node(node* prev, node* next) : mPrev(prev), mNext(next) {}

    node* get_prev() const { return mPrev; }
    node* get_next() const { return mNext; }
    void set_prev(node* prev) { mPrev = prev; }
    void set_next(node* next) { mNext = next; }
    T* get_value() { return reinterpret_cast< T* >(&mItem); }
    const T* get_value() const { return reinterpret_cast< const T* >(&mItem); }
  };

  node* create_node(node* prev, node* next, const T& val) {
    node* n;
    mAllocator.allocate(n, 1);
    new (n) node(prev, next);
    new (n->mItem) T(val);
    return n;
  }

  node* do_insert_before(node* n, const T& val) {
    node* const nn = create_node(n->mPrev, n, val);
    if (n == mStart) {
      mStart = nn;
    }
    nn->get_prev()->set_next(nn);
    nn->get_next()->set_prev(nn);
    ++mCount;

    return nn;
  }

  iterator insert(const iterator& pos, const T& val);

  template < typename InputIterator >
  void insert(const iterator& pos, InputIterator first, InputIterator last);

  void destroy() {
#if RSTL_VERSION >= RSTL_GM8P_00
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
    node* it = mStart;
    while (it != mEnd) {
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
      this->current = this->current->mNext;
      return *this;
    }
    const_iterator operator++(int) const { return const_iterator(this->current->mNext); }
    const_iterator& operator--() {
      this->current = this->current->mPrev;
      return *this;
    }
    const_iterator operator--(int) const { return const_iterator(this->current->mPrev); }
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
      this->current = this->current->mNext;
      return *this;
    }
    iterator operator++(int) {
      node* cur = this->current;
      this->current = this->current->mNext;
      return cur;
    }
    iterator& operator--() {
      this->current = this->current->mPrev;
      return *this;
    }
    iterator operator--(int) { return iterator(this->curent->mPrev); }
    T* get_pointer() const { return this->current->get_value(); }
    T& operator*() const { return *this->current->get_value(); }
    T* operator->() const { return this->current->get_value(); }
    bool operator==(const iterator& other) const { return this->current == other.current; }
    bool operator!=(const iterator& other) const { return this->current != other.current; }
  };

private:
  Alloc mAllocator;
  node* mStart;
  node* mEnd;
  ALIGNAS(node) node* mEmpty_prev;
  node* mEmpty_next;
  int mCount;
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
  node* cur = mStart;
  while (cur != mEnd) {
    node* it = cur;
    node* next = cur->get_next();
    cur = next;
    it->get_value()->~T();
    mAllocator.deallocate(it);
  }
}

template < typename T, typename Alloc >
typename list< T, Alloc >::node* list< T, Alloc >::do_erase(node* node) {
  typename list< T, Alloc >::node* result = node->get_next();
  if (node == mStart) {
    mStart = node->get_next();
  }
  node->get_prev()->set_next(node->get_next());
  node->get_next()->set_prev(node->get_prev());
  node->get_value()->~T();
  mAllocator.deallocate(node);
  mCount--;
  return result;
}

template < typename T, typename Alloc >
void list< T, Alloc >::exchange(node* nodeA, node* nodeB) {
  if (nodeA == nodeB) {
    return;
  }

  if (nodeA == this->mStart) {
    this->mStart = nodeB;
  } else if (nodeB == this->mStart) {
    this->mStart = nodeA;
  }

  node* nodeANext = nodeA->mNext;
  node* nodeAPrev = nodeA->mPrev;
  node* nodeBNext = nodeB->mNext;
  node* nodeBPrev = nodeB->mPrev;

  if (nodeBNext != nodeA) {
    nodeBNext->mPrev = nodeA;
    nodeA->mNext = nodeBNext;
    nodeAPrev->mNext = nodeB;
    nodeB->mPrev = nodeAPrev;
  } else {
    nodeA->mNext = nodeB;
    nodeB->mPrev = nodeA;
  }

  if (nodeBPrev != nodeA) {
    nodeBPrev->mNext = nodeA;
    nodeA->mPrev = nodeBPrev;
    nodeANext->mPrev = nodeB;
    nodeB->mNext = nodeANext;
  } else {
    nodeB->mNext = nodeA;
    nodeA->mPrev = nodeB;
  }
}

} // namespace rstl

#endif // _RSTL_LIST
