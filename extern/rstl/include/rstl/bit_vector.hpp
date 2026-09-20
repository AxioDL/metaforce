#ifndef _RSTL_BIT_VECTOR
#define _RSTL_BIT_VECTOR

#include "rstl/vector.hpp"

namespace rstl {
template < typename Alloc = rmemory_allocator >
class bit_vector {
  typedef vector< uint, Alloc > storage_type;

public:
  typedef Alloc allocator_type;

  class bit_reference {
  public:
    bit_reference(typename storage_type::iterator word, int bit) : mWord(word), mMask(1 << bit) {}
    operator bool() const { return (mMask & *mWord.get_pointer()) != 0; }
    bit_reference& operator=(bool value) {
      if (value) {
        *mWord |= mMask;
      } else {
        *mWord &= ~mMask;
      }
      return *this;
    }

  private:
    typename storage_type::iterator mWord;
    int mMask;
  };

  class const_iterator {
  public:
    const_iterator(typename storage_type::const_iterator data, int bit, int)
    : mData(data), mCurrentBit(bit) {}
    int get_current_bit() const { return mCurrentBit; }

  protected:
    typename storage_type::const_iterator mData;
    int mCurrentBit;
  };

  class iterator : public const_iterator {
  public:
    iterator(typename storage_type::iterator data, int bit, int offset)
    : const_iterator(data, bit, offset), mData(data) {}

  private:
    typename storage_type::iterator mData;
  };

  bit_vector() : mSize(0) {}
  bit_vector(int count, bool value) : mSize(0) {
    reserve(count);
    insert(begin(), count, value);
  }

  bit_vector& operator=(const bit_vector& other) {
    mSize = other.mSize;
    mData = other.mData;
    return *this;
  }

  int size() const { return mSize; }
  iterator begin() { return iterator(mData.begin(), 0, 0); }
  iterator end() { return iterator(mData.begin(), mSize, 0); }
  void reserve(int count) { mData.reserve(get_data_size(count)); }
  bit_reference at(int bit) {
    return bit_reference(mData.begin() + get_real_index(bit), bit % 32);
  }
  bit_reference operator[](int bit) { return at(bit); }
  void push_back(bool value) {
    if (mSize % 32 == 0) {
      mData.push_back(0u);
    }
    set_bit(mSize++, value);
  }
  void insert(iterator at, int count, bool value) {
    int bit = at.get_current_bit();
    make_room(bit, count);
    while (count-- > 0) {
      set_bit(bit++, value);
    }
  }

  bool get_bit(int bit) { return (mData[get_real_index(bit)] & get_real_bit_mask(bit)) != 0; }
  void set_bit(int bit, bool value) {
    if (value) {
      mData[get_real_index(bit)] |= get_real_bit_mask(bit);
    } else {
      mData[get_real_index(bit)] &= ~get_real_bit_mask(bit);
    }
  }
  int get_real_index(int bit) { return bit / 32; }
  uint get_real_bit_mask(int bit) { return 1 << (bit % 32); }
  int get_data_size(int count) { return count / 32 + (count % 32 ? 1 : 0); }
  void make_room(int bit, int count) {
    int available = mData.size() * 32 - mSize;
    while (available < count) {
      mData.push_back(0u);
      available += 32;
    }
    for (int i = mSize - 1; i >= bit; --i) {
      set_bit(i + count, get_bit(i));
    }
    mSize += count;
  }

  void PutTo(COutputStream& out) const { mData.PutTo(out); }

private:
  int mSize;
  storage_type mData;
};
} // namespace rstl
#endif // _RSTL_BIT_VECTOR
