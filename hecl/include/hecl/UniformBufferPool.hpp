#ifndef HECL_UNIFORMBUFFERPOOL_HPP
#define HECL_UNIFORMBUFFERPOOL_HPP

#include <boo/boo.hpp>
#include <vector>
#include <cstdlib>
#include <atomic>
#include "BitVector.hpp"

namespace hecl
{

#define HECL_UBUFPOOL_ALLOCATION_BLOCK 262144

/** This class provides a uniform structure for packing instanced uniform-buffer
 *  data with consistent stride into a vector of 256K 'Buckets'.
 *
 *  This results in a space-efficient way of managing GPU data of things like UI
 *  widgets. These can potentially have numerous binding instances, so this avoids
 *  allocating a full GPU buffer object for each. */
template <typename UniformStruct>
class UniformBufferPool
{
    /* Resolve div_t type using ssize_t as basis */
#if _WIN32
    using IndexTp = SSIZE_T;
#else
    using IndexTp = ssize_t;
#endif
    struct InvalidTp {};
    using DivTp = std::conditional_t<std::is_same<IndexTp, long long>::value, std::lldiv_t,
                  std::conditional_t<std::is_same<IndexTp, long>::value, std::ldiv_t,
                  std::conditional_t<std::is_same<IndexTp, int>::value, std::div_t, InvalidTp>>>;
    static_assert(!std::is_same<DivTp, InvalidTp>::value, "unsupported IndexTp for DivTp resolution");

    /** Size of single element, rounded up to 256-multiple */
    static constexpr IndexTp m_stride = ROUND_UP_256(sizeof(UniformStruct));
    static_assert(m_stride <= HECL_UBUFPOOL_ALLOCATION_BLOCK, "Stride too large for uniform pool");

    /** Number of rounded elements per 256K bucket */
    static constexpr IndexTp m_countPerBucket = HECL_UBUFPOOL_ALLOCATION_BLOCK / m_stride;

    /** Buffer size per bucket (ideally 256K) */
    static constexpr IndexTp m_sizePerBucket = m_stride * m_countPerBucket;

    /** BitVector indicating free allocation blocks */
    hecl::llvm::BitVector m_freeBlocks;

    /** Efficient way to get bucket and block simultaneously */
    DivTp getBucketDiv(IndexTp idx) const { return std::div(idx, m_countPerBucket); }

    /** Buffer pool token */
    boo::GraphicsBufferPoolToken m_token;

    /** Private bucket info */
    struct Bucket
    {
        boo::IGraphicsBufferD* buffer;
        uint8_t* cpuBuffer = nullptr;
        std::atomic_size_t useCount = {};
        bool dirty = false;
        Bucket() = default;
        Bucket(const Bucket& other) = delete;
        Bucket& operator=(const Bucket& other) = delete;
        Bucket(Bucket&& other) = default;
        Bucket& operator=(Bucket&& other) = default;

        void updateBuffer()
        {
            if (buffer)
                buffer->unmap();
            cpuBuffer = nullptr;
            dirty = false;
        }

        void increment(UniformBufferPool& pool)
        {
            if (useCount.fetch_add(1) == 0)
                buffer = pool.m_token.newPoolBuffer(boo::BufferUse::Uniform,
                                                    pool.m_stride, pool.m_countPerBucket);
        }

        void decrement(UniformBufferPool& pool)
        {
            if (useCount.fetch_sub(1) == 1)
            {
                pool.m_token.deletePoolBuffer(buffer);
                buffer = nullptr;
            }
        }
    };
    std::vector<std::unique_ptr<Bucket>> m_buckets;

public:
    /** User block-owning token */
    class Token
    {
        friend class UniformBufferPool;
        UniformBufferPool* m_pool = nullptr;
        IndexTp m_index = -1;
        DivTp m_div;
        Token(UniformBufferPool* pool)
        : m_pool(pool)
        {
            auto& freeSpaces = pool->m_freeBlocks;
            int idx = freeSpaces.find_first();
            if (idx == -1)
            {
                pool->m_buckets.push_back(std::make_unique<Bucket>());
                m_index = freeSpaces.size();
                freeSpaces.resize(freeSpaces.size() + pool->m_countPerBucket, true);
            }
            else
            {
                m_index = idx;
            }
            freeSpaces.reset(m_index);
            m_div = pool->getBucketDiv(m_index);

            Bucket& bucket = *m_pool->m_buckets[m_div.quot];
            bucket.increment(*m_pool);
        }

    public:
        Token() = default;
        Token(const Token& other) = delete;
        Token& operator=(const Token& other) = delete;
        Token& operator=(Token&& other)
        {
            m_pool = other.m_pool;
            m_index = other.m_index;
            m_div = other.m_div;
            other.m_index = -1;
            return *this;
        }
        Token(Token&& other)
        : m_pool(other.m_pool), m_index(other.m_index),
          m_div(other.m_div)
        {
            other.m_index = -1;
        }

        ~Token()
        {
            if (m_index != -1)
            {
                m_pool->m_freeBlocks.set(m_index);
                Bucket& bucket = *m_pool->m_buckets[m_div.quot];
                bucket.decrement(*m_pool);
            }
        }

        UniformStruct& access()
        {
            Bucket& bucket = *m_pool->m_buckets[m_div.quot];
            if (!bucket.cpuBuffer)
                bucket.cpuBuffer = reinterpret_cast<uint8_t*>(bucket.buffer->map(m_sizePerBucket));
            bucket.dirty = true;
            return reinterpret_cast<UniformStruct&>(bucket.cpuBuffer[m_div.rem * m_pool->m_stride]);
        }

        std::pair<boo::IGraphicsBufferD*, IndexTp> getBufferInfo() const
        {
            Bucket& bucket = *m_pool->m_buckets[m_div.quot];
            return {bucket.buffer, m_div.rem * m_pool->m_stride};
        }

        operator bool() const { return m_pool != nullptr && m_index != -1; }
    };

    UniformBufferPool() = default;
    UniformBufferPool(const UniformBufferPool& other) = delete;
    UniformBufferPool& operator=(const UniformBufferPool& other) = delete;

    /** Load dirty buffer data into GPU */
    void updateBuffers()
    {
        for (auto& bucket : m_buckets)
            if (bucket->dirty)
                bucket->updateBuffer();
    }

    /** Allocate free block into client-owned Token */
    Token allocateBlock(boo::IGraphicsDataFactory* factory)
    {
        if (!m_token)
            m_token = factory->newBufferPool();
        return Token(this);
    }

    void doDestroy() { m_token.doDestroy(); }
};

}

#endif // HECL_UNIFORMBUFFERPOOL_HPP
