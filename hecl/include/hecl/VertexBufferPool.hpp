#pragma once

#include <boo/boo.hpp>
#include <vector>
#include <cstdlib>
#include <atomic>
#include "BitVector.hpp"

namespace hecl {

#define HECL_VBUFPOOL_ALLOCATION_BLOCK 262144

/** This class provides a uniform structure for packing instanced vertex-buffer
 *  data with consistent stride into a vector of 256K 'Buckets'.
 *
 *  This results in a space-efficient way of managing GPU data of things like UI
 *  widgets. These can potentially have numerous binding instances, so this avoids
 *  allocating a full GPU buffer object for each. */
template <typename VertStruct>
class VertexBufferPool {
public:
  /* Resolve div_t type using ssize_t as basis */
#if _WIN32
  using IndexTp = SSIZE_T;
#else
  using IndexTp = ssize_t;
#endif
private:
  struct InvalidTp {};
  using DivTp = std::conditional_t<
      std::is_same<IndexTp, long long>::value, std::lldiv_t,
      std::conditional_t<std::is_same<IndexTp, long>::value, std::ldiv_t,
                         std::conditional_t<std::is_same<IndexTp, int>::value, std::div_t, InvalidTp>>>;
  static_assert(!std::is_same<DivTp, InvalidTp>::value, "unsupported IndexTp for DivTp resolution");

  /** Size of single element */
  static constexpr IndexTp m_stride = sizeof(VertStruct);
  static_assert(m_stride <= HECL_VBUFPOOL_ALLOCATION_BLOCK, "Stride too large for vertex pool");

  /** Number of elements per 256K bucket */
  static constexpr IndexTp m_countPerBucket = HECL_VBUFPOOL_ALLOCATION_BLOCK / m_stride;

  /** Buffer size per bucket (ideally 256K) */
  static constexpr IndexTp m_sizePerBucket = m_stride * m_countPerBucket;

  /** BitVector indicating free allocation elements */
  hecl::llvm::BitVector m_freeElements;

  /** Efficient way to get bucket and element simultaneously */
  DivTp getBucketDiv(IndexTp idx) const { return std::div(idx, m_countPerBucket); }

  /** Factory pointer for building additional buffers */
  boo::IGraphicsDataFactory* m_factory = nullptr;

  /** Private bucket info */
  struct Bucket {
    boo::ObjToken<boo::IGraphicsBufferD> buffer;
    uint8_t* cpuBuffer = nullptr;
    std::atomic_size_t useCount = {};
    bool dirty = false;
    Bucket() = default;
    Bucket(const Bucket& other) = delete;
    Bucket& operator=(const Bucket& other) = delete;
    Bucket(Bucket&& other) = delete;
    Bucket& operator=(Bucket&& other) = delete;

    void updateBuffer() {
      if (cpuBuffer) {
        buffer->unmap();
        cpuBuffer = nullptr;
      }
      dirty = false;
    }

    void increment(VertexBufferPool& pool) {
      if (useCount.fetch_add(1) == 0)
        buffer = pool.m_factory->newPoolBuffer(boo::BufferUse::Vertex, pool.m_stride, pool.m_countPerBucket BooTrace);
    }

    void decrement(VertexBufferPool& pool) {
      if (useCount.fetch_sub(1) == 1) {
        if (cpuBuffer) {
          buffer->unmap();
          cpuBuffer = nullptr;
        }
        buffer.reset();
      }
    }
  };
  std::vector<std::unique_ptr<Bucket>> m_buckets;

public:
  /** User element-owning token */
  class Token {
    friend class VertexBufferPool;
    VertexBufferPool* m_pool = nullptr;
    IndexTp m_index = -1;
    IndexTp m_count = 0;
    DivTp m_div;
    Token(VertexBufferPool* pool, IndexTp count) : m_pool(pool), m_count(count) {
      assert(count <= pool->m_countPerBucket && "unable to fit in bucket");
      auto& freeSpaces = pool->m_freeElements;
      int idx = freeSpaces.find_first_contiguous(count, pool->m_countPerBucket);
      if (idx == -1) {
        pool->m_buckets.push_back(std::make_unique<Bucket>());
        m_index = freeSpaces.size();
        freeSpaces.resize(freeSpaces.size() + pool->m_countPerBucket, true);
      } else {
        m_index = idx;
      }
      freeSpaces.reset(m_index, m_index + count);
      m_div = pool->getBucketDiv(m_index);

      Bucket& bucket = *m_pool->m_buckets[m_div.quot];
      bucket.increment(*pool);
    }

  public:
    Token() = default;
    Token(const Token& other) = delete;
    Token& operator=(const Token& other) = delete;
    Token& operator=(Token&& other) {
      m_pool = other.m_pool;
      m_index = other.m_index;
      m_count = other.m_count;
      m_div = other.m_div;
      other.m_index = -1;
      return *this;
    }
    Token(Token&& other) : m_pool(other.m_pool), m_index(other.m_index), m_count(other.m_count), m_div(other.m_div) {
      other.m_index = -1;
    }

    ~Token() {
      if (m_index != -1) {
        m_pool->m_freeElements.set(m_index, m_index + m_count);
        Bucket& bucket = *m_pool->m_buckets[m_div.quot];
        bucket.decrement(*m_pool);
      }
    }

    VertStruct* access() {
      Bucket& bucket = *m_pool->m_buckets[m_div.quot];
      if (!bucket.cpuBuffer)
        bucket.cpuBuffer = reinterpret_cast<uint8_t*>(bucket.buffer->map(m_sizePerBucket));
      bucket.dirty = true;
      return reinterpret_cast<VertStruct*>(&bucket.cpuBuffer[m_div.rem * m_pool->m_stride]);
    }

    std::pair<boo::ObjToken<boo::IGraphicsBufferD>, IndexTp> getBufferInfo() const {
      Bucket& bucket = *m_pool->m_buckets[m_div.quot];
      return {bucket.buffer, m_div.rem};
    }

    operator bool() const { return m_pool != nullptr && m_index != -1; }
  };

  VertexBufferPool() = default;
  VertexBufferPool(const VertexBufferPool& other) = delete;
  VertexBufferPool& operator=(const VertexBufferPool& other) = delete;

  /** Load dirty buffer data into GPU */
  void updateBuffers() {
    for (auto& bucket : m_buckets)
      if (bucket->dirty)
        bucket->updateBuffer();
  }

  /** Allocate free block into client-owned Token */
  Token allocateBlock(boo::IGraphicsDataFactory* factory, IndexTp count) {
    m_factory = factory;
    return Token(this, count);
  }

  void doDestroy() {
    for (auto& bucket : m_buckets)
      bucket->buffer.reset();
  }

  static constexpr IndexTp bucketCapacity() { return m_countPerBucket; }
};

} // namespace hecl
