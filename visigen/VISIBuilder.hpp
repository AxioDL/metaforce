#ifndef _DNACOMMON_VISIBUILDER_HPP_
#define _DNACOMMON_VISIBUILDER_HPP_

#include "VISIRenderer.hpp"
#include "zeus/CAABox.hpp"
#include "hecl/extern/boo/xxhash/xxhash.h"
#include "athena/MemoryWriter.hpp"
#include "hecl/hecl.hpp"
#include <unordered_map>

namespace std
{
template <> struct hash<zeus::CVector3f>
{
    size_t operator()(const zeus::CVector3f& val) const noexcept
    {
        return XXH64(val.v, 12, 0);
    }
};
}

struct VISIBuilder
{
    struct Leaf
    {
        std::vector<uint8_t> bits;
        void setBit(size_t bit)
        {
            size_t byte = bit / 8;
            if (byte >= bits.size())
                bits.resize(byte + 1);
            bits[byte] |= 1 << (bit & 0x7);
        }
        void setLightEnum(size_t bit, EPVSVisSetState state)
        {
            size_t byte0 = bit / 8;
            size_t byte1 = (bit + 1) / 8;
            if (byte1 >= bits.size())
                bits.resize(byte1 + 1);

            if (byte0 == byte1)
            {
                bits[byte0] |= int(state) << (bit & 0x7);
            }
            else
            {
                bits[byte0] |= (int(state) << 7) & 0x1;
                bits[byte1] |= (int(state) >> 1) & 0x1;
            }
        }
        bool operator==(const Leaf& other) const
        {
            if (bits.size() != other.bits.size())
                return false;
            if (memcmp(bits.data(), other.bits.data(), bits.size()))
                return false;
            return true;
        }
        Leaf& operator|=(const Leaf& other)
        {
            if (bits.size() < other.bits.size())
                bits.resize(other.bits.size());
            for (int i=0 ; i<other.bits.size() ; ++i)
                bits[i] |= other.bits[i];
            return *this;
        }
        operator bool() const { return bits.size() != 0; }

        void write(athena::io::MemoryWriter& w, size_t leafBytes) const
        {
            for (size_t i=0 ; i<leafBytes ; ++i)
            {
                if (i < bits.size())
                    w.writeUByte(bits[i]);
                else
                    w.writeUByte(0);
            }
        }
    };

    class PVSRenderCache
    {
        friend struct VISIBuilder;
        VISIRenderer& m_renderer;
        std::unordered_map<zeus::CVector3f, std::unique_ptr<Leaf>> m_cache;
        size_t m_lightMetaBit;
    public:
        PVSRenderCache(VISIRenderer& renderer);
        const Leaf& GetLeaf(const zeus::CVector3f& vec);
    } renderCache;

    class Progress
    {
        float m_prog = 0.f;
        FPercent m_updatePercent;
    public:
        void report(int divisions);
        Progress(FPercent updatePercent) : m_updatePercent(updatePercent) {}
    };

    struct Node
    {
        std::vector<Node> childNodes;
        size_t childRelOffs[8] = {};
        Leaf leaf;
        uint8_t flags = 0;

        void buildChildren(int level, int divisions, const zeus::CAABox& curAabb,
                           PVSRenderCache& rc, Progress& prog, const bool& terminate);
        void calculateSizesAndOffs(size_t& cur, size_t leafSz);
        void writeNodes(athena::io::MemoryWriter& w, size_t leafBytes) const;

        bool operator==(const Node& other) const
        {
            if ((flags & 0x7) || (other.flags & 0x7))
                return false;
            return leaf == other.leaf;
        }
    } rootNode;

    std::vector<uint8_t> build(const zeus::CAABox& fullAabb,
                               size_t modelCount,
                               const std::vector<VISIRenderer::Entity>& entities,
                               const std::vector<VISIRenderer::Light>& lights,
                               size_t layer2LightCount,
                               FPercent updatePercent);

    VISIBuilder(VISIRenderer& renderer) : renderCache(renderer) {}
};

#endif // _DNACOMMON_VISIBUILDER_HPP_
