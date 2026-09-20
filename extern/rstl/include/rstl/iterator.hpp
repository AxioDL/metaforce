#ifndef _RSTL_ITERATOR
#define _RSTL_ITERATOR

#include <stddef.h>

namespace rstl {

struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

template < typename It >
struct iterator_traits {
  typedef typename It::value_type value_type;
  typedef typename It::difference_type difference_type;
  typedef typename It::iterator_category iterator_category;
};

template < typename T >
struct iterator_traits< T* > {
  typedef T value_type;
  typedef ptrdiff_t difference_type;
  typedef random_access_iterator_tag iterator_category;
};

template < typename It >
typename iterator_traits< It >::difference_type __distance(It first, It last,
                                                           forward_iterator_tag) {
  typename iterator_traits< It >::difference_type result = 0;
  It it = first;
  while (it != last) {
    ++result;
    ++it;
  }
  return result;
}

template < typename It >
typename iterator_traits< It >::difference_type __distance(It first, It last,
                                                           random_access_iterator_tag) {
  return last - first;
}

template < typename It >
typename iterator_traits< It >::difference_type distance(It first, It last) {
  return __distance(first, last, typename iterator_traits< It >::iterator_category());
}

template < typename It, typename S >
void __advance(It& it, S count, forward_iterator_tag) {
  while (count > 0) {
    ++it;
  }
}

template < typename It, typename S >
void __advance(It& it, S count, bidirectional_iterator_tag) {
  if (count > 0) {
    for (S i = 0; i < count; ++i) {
      ++it;
    }
  } else {
    for (S i = 0; i > count; --i) {
      --it;
    }
  }
}

template < typename It, typename S >
void __advance(It& it, S count, random_access_iterator_tag) {
  it += count;
}

template < typename It, typename S >
void advance(It& it, S count) {
  __advance(it, count, typename iterator_traits< It >::iterator_category());
}

template < typename It, typename S >
It advance_iterator(It it, S count) {
  It result = it;
  advance(result, count);
  return result;
}

} // namespace rstl

#endif // _RSTL_ITERATOR
