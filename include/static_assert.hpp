// C++98 static assert

#define DECLARE_FULL_SIZE_FOR(cls, size) \
  enum { cls##_FULL_SIZE = (size) };

#define DECLARE_SIZES_FOR(cls, parent, partial_size) \
  enum { cls##_PARTIAL_SIZE = (partial_size) }; \
  enum { cls##_FULL_SIZE = parent##_FULL_SIZE + cls##_PARTIAL_SIZE };

#ifdef __MWERKS__
struct false_type {
  static const int value = 0;
};

struct true_type {
  static const int value = 1;
};

template < int A, int B >
struct _n_is_equal : false_type {};

template < int A >
struct _n_is_equal< A, A > : true_type {};

template < class T, int N >
struct check_sizeof : _n_is_equal< sizeof(T), N > {};

#ifndef offsetof
typedef unsigned long size_t;
#define offsetof(type, member) ((size_t) & (((type*)0)->member))
#endif
#define CHECK_SIZEOF(cls, size) extern int cls##_check[check_sizeof< cls, size >::value];
#define CHECK_CHILD_SIZEOF(cls, parent, partial) DECLARE_SIZES_FOR(cls, parent, partial); extern int cls##_check[check_sizeof< cls, (parent##_FULL_SIZE + cls##_PARTIAL_SIZE) >::value];
#define NESTED_CHECK_SIZEOF(parent, cls, size)                                                     \
  extern int cls##_check[check_sizeof< parent::cls, size >::value];
#define CHECK_OFFSETOF(cls, member, offset)                                                        \
  extern int cls##_check_offset##[_n_is_equal< offsetof(cls, member), offset >::value];
#elif defined(__clang__) && defined(__powerpc__)      // Enable for clangd
#pragma clang diagnostic ignored "-Wc11-extensions"   // Allow _Static_assert
#pragma clang diagnostic ignored "-Wc++17-extensions" // Allow _Static_assert without message
#define CHECK_SIZEOF(cls, size) _Static_assert(sizeof(cls) == size);
#define CHECK_CHILD_SIZEOF(cls, parent, partial) DECLARE_SIZES_FOR(cls, parent, partial); _Static_assert(sizeof(cls) == (parent##_FULL_SIZE + cls##_PARTIAL_SIZE));
#define NESTED_CHECK_SIZEOF(parent, cls, size) _Static_assert(sizeof(parent::cls) == size);
#define CHECK_OFFSETOF(cls, member, offset) _Static_assert(offsetof(cls, member) == offset);
#else
#define CHECK_SIZEOF(cls, size)
#define CHECK_CHILD_SIZEOF(cls, parent, partial)
#define NESTED_CHECK_SIZEOF(parent, cls, size)
#define CHECK_OFFSETOF(cls, member, offset)
#endif
