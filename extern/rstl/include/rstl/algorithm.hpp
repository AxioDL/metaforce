#ifndef _RSTL_ALGORITHM
#define _RSTL_ALGORITHM

#include "rstl/functional.hpp"
#include "rstl/pair.hpp"
#include "rstl/pointer_iterator.hpp"

namespace rstl {
template < typename It, typename T >
int count(It first, It last, const T& val) {
  int result = 0;
  for (; first != last; ++first) {
    if (*first == val) {
      ++result;
    }
  }
  return result;
}

template < class It, class T >
inline It find(It first, It last, const T& val) {
  while (first != last && !(*first == val))
    ++first;
  return first;
}

template < typename T >
inline void swap(T& a, T& b) {
  T tmp(a);
  a = b;
  b = tmp;
}

template < typename I1, typename I2 >
inline void iter_swap(I1 a, I2 b) {
  typename iterator_traits< I1 >::value_type tmp = *a;
  *a = *b;
  *b = tmp;
}

template < typename T, class Cmp >
void __sort3(T& a, T& b, T& c, const Cmp comp) {
  if (comp(b, a)) {
    swap(a, b);
  }
  if (comp(c, b)) {
    T tmp(c);
    c = b;
    if (comp(tmp, a)) {
      b = a;
      a = tmp;
    } else {
      b = tmp;
    }
  }
}

template < typename It, class Cmp >
void __insertion_sort(It first, It last, Cmp cmp) {
  It next = first;
  for (++next; next < last; ++next) {
    typename iterator_traits< It >::value_type value = *next;

    It t1 = next - 1;
    It t2 = next;
    while (first < t2 && cmp(value, *t1)) {
      *t2 = *t1;
      --t2;
      --t1;
    }
    *t2 = value;
  }
}

template < typename It, class Cmp >
void sort(It first, It last, Cmp cmp) {
  const typename iterator_traits< It >::difference_type count = last - first;
  if (count <= 1) {
    return;
  }
  if (count <= 20) {
    __insertion_sort(first, last, cmp);
    return;
  }

  It mid = first + count / 2;
  It end = last - 1;
  __sort3(*first, *mid, *end, cmp);

  typename iterator_traits< It >::value_type pivot = *mid;
  It it = first + 1;
  --end;

  while (true) {
    while (cmp(*it, pivot)) {
      ++it;
    }
    while (cmp(pivot, *end)) {
      --end;
    }
    if (it >= end) {
      break;
    }
    iter_swap(it, end);
    ++it;
    --end;
  }

  sort(first, it, cmp);
  sort(it, last, cmp);
}

template < typename It, typename T, typename Cmp >
It lower_bound(It start, It end, const T& value, Cmp cmp) {
  int dist = distance(start, end);
  It it = start;
  while (dist > 0) {
    int halfDist = dist / 2;
    it = start;
    advance(it, halfDist);
    if (cmp(*it, value)) {
      start = it;
      ++start;
      dist = (dist - halfDist) - 1;
    } else {
      dist = halfDist;
    }
  }
  return start;
}

template < typename T >
void __sort3(T& a, T& b, T& c) {
  if (b < a) {
    swap(a, b);
  }
  if (c < b) {
    T tmp(c);
    c = b;
    if (tmp < a) {
      b = a;
      a = tmp;
    } else {
      b = tmp;
    }
  }
}

template < typename It >
void __insertion_sort(It first, It last) {
  It next = first;
  for (++next; next < last; ++next) {
    typename iterator_traits< It >::value_type value = *next;
    It t1 = next - 1;
    It t2 = next;
    while (first < t2 && value < *t1) {
      *t2 = *t1;
      --t2;
      --t1;
    }
    *t2 = value;
  }
}

template < typename It >
void sort(It first, It last) {
  const typename iterator_traits< It >::difference_type count = last - first;
  if (count <= 1) {
    return;
  }
  if (count <= 20) {
    __insertion_sort(first, last);
    return;
  }
  It mid = first + count / 2;
  It end = last - 1;
  __sort3(*first, *mid, *end);
  typename iterator_traits< It >::value_type pivot = *mid;
  It it = first + 1;
  --end;
  while (true) {
    while (*it < pivot) {
      ++it;
    }
    while (pivot < *end) {
      --end;
    }
    if (it >= end) {
      break;
    }
    iter_swap(it, end);
    ++it;
    --end;
  }
  sort(first, it);
  sort(it, last);
}

template < typename It, typename T >
It lower_bound(It start, It end, const T& value) {
  int dist = distance(start, end);
  It it = start;
  while (dist > 0) {
    int halfDist = dist / 2;
    it = start;
    advance(it, halfDist);
    if (*it < value) {
      start = it;
      ++start;
      dist = (dist - halfDist) - 1;
    } else {
      dist = halfDist;
    }
  }
  return start;
}

template < typename Vec >
typename Vec::const_iterator lower_bound_const(typename Vec::const_iterator start,
                                               typename Vec::const_iterator end,
                                               const typename Vec::value_type& value) {
  int dist = distance(start, end);
  typename Vec::const_iterator it = start;
  while (dist > 0) {
    int halfDist = dist / 2;
    it = start;
    advance(it, halfDist);
    if (*it < value) {
      start = it;
      ++start;
      dist = (dist - halfDist) - 1;
    } else {
      dist = halfDist;
    }
  }
  return start;
}

template < typename It, typename T >
It upper_bound(It start, It end, const T& value) {
  int dist = distance(start, end);
  It it = start;
  while (dist > 0) {
    int halfDist = dist / 2;
    it = start;
    advance(it, halfDist);
    if (value < *it) {
      dist = halfDist;
    } else {
      start = it;
      ++start;
      dist = (dist - halfDist) - 1;
    }
  }
  return start;
}

template < typename Vec >
typename Vec::iterator lower_bound(typename Vec::iterator start, typename Vec::iterator end,
                                   const typename Vec::value_type& value) {
  int dist = distance(start, end);
  typename Vec::iterator it = start;
  while (dist > 0) {
    int halfDist = dist / 2;
    it = start;
    advance(it, halfDist);
    if (*it < value) {
      start = it;
      ++start;
      dist = (dist - halfDist) - 1;
    } else {
      dist = halfDist;
    }
  }
  return start;
}

template < typename It, typename T, typename Cmp >
inline It binary_find(It start, It end, const T& value, Cmp cmp) {
  It lower = lower_bound(start, end, value, cmp);
  return It((lower != end && !cmp(value, *lower)) ? lower : end);
}

template < typename It, typename T >
inline It binary_find(It start, It end, const T& value) {
  It lower = lower_bound(start, end, value);
  bool found = lower != end && !(value < *lower);
  return found ? lower : end;
}

template < typename Vec >
inline typename Vec::const_iterator binary_find_const(typename Vec::const_iterator start,
                                                      typename Vec::const_iterator end,
                                                      const typename Vec::value_type& value) {
  typename Vec::const_iterator lower = lower_bound_const< Vec >(start, end, value);
  bool found = (lower != end && !(value < *lower));
  return found ? lower : end;
}

template < typename Vec >
inline typename Vec::iterator binary_find(typename Vec::iterator start, typename Vec::iterator end,
                                          const typename Vec::value_type& value) {
  typename Vec::iterator lower = lower_bound< Vec >(start, end, value);
  bool found = (lower != end && !(value < *lower));
  return found ? lower : end;
}

template < typename T, typename Cmp >
class pair_sorter_finder;

template < typename K, typename V, typename Cmp >
class pair_sorter_finder< pair< K, V >, Cmp > {
public:
  typedef K key_type;
  Cmp cmp;
  pair_sorter_finder(const Cmp& cmp) : cmp(cmp) {}

  bool operator()(const K& a, const pair< K, V >& b) const;
  /* {
    return cmp(a, b.first);
  }*/

  bool operator()(const pair< K, V >& a, const K& b) const;
  /* {
    return cmp(a.first, b);
  }*/

  bool operator()(const pair< K, V >& a, const pair< K, V >& b) const;
};

template < typename T >
inline pair_sorter_finder< typename T::value_type,
                           less< typename select1st< typename T::value_type >::value_type > >
default_pair_sorter_finder() {
  less< typename select1st< typename T::value_type >::value_type > l;
  pair_sorter_finder< typename T::value_type,
                      less< typename select1st< typename T::value_type >::value_type > >
      a(l);
  return a;
}

template < typename K, typename V, typename Cmp >
inline bool pair_sorter_finder< pair< K, V >, Cmp >::operator()(const K& a,
                                                                const pair< K, V >& b) const {
  return cmp(a, b.first);
}

template < typename K, typename V, typename Cmp >
inline bool pair_sorter_finder< pair< K, V >, Cmp >::operator()(const pair< K, V >& a,
                                                                const K& b) const {
  return cmp(a.first, b);
}

template < typename K, typename V, typename Cmp >
inline bool pair_sorter_finder< pair< K, V >, Cmp >::operator()(const pair< K, V >& a,
                                                                const pair< K, V >& b) const {
  return cmp(a.first, b.first);
}

template < typename T >
typename T::const_iterator
find_by_key(const T& container,
            const typename select1st< typename T::value_type >::value_type& key);

template < typename T >
typename T::const_iterator inline find_by_key(
    const T& container, const typename select1st< typename T::value_type >::value_type& key) {
  return binary_find(container.begin(), container.end(), key,
    pair_sorter_finder< typename T::value_type, less< typename select1st< typename T::value_type >::value_type > >(
      less< typename select1st< typename T::value_type >::value_type >()));
}

template < typename T, class Cmp >
typename T::const_iterator inline find_by_key(
    const T& container, const typename select1st< typename T::value_type >::value_type& key,
    const Cmp& cmp) {
  return binary_find(container.begin(), container.end(), key,
                     pair_sorter_finder< typename T::value_type, Cmp >(cmp));
}

template < typename T >
typename T::iterator
find_by_key_nc(T& container, const typename select1st< typename T::value_type >::value_type& key);

template < typename T >
typename T::iterator inline find_by_key_nc(
    T& container, const typename select1st< typename T::value_type >::value_type& key) {
  return binary_find(container.begin(), container.end(), key,
    pair_sorter_finder< typename T::value_type, less< typename select1st< typename T::value_type >::value_type > >(
      less< typename select1st< typename T::value_type >::value_type >()));
}

template < typename T >
inline void sort_by_key(T& container) {
  less< typename select1st< typename T::value_type >::value_type > cmp;
  sort_by_key(container, cmp);
}

template < typename T, class Cmp >
inline void sort_by_key(T& container, const Cmp& cmp) {
  sort(container.begin(), container.end(), pair_sorter_finder< typename T::value_type, Cmp >(cmp));
}

} // namespace rstl

#endif // _RSTL_ALGORITHM
