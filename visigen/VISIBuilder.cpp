#include "VISIBuilder.hpp"

#define VISI_MAX_LEVEL 10
#define VISI_MIN_LENGTH 8.0

static logvisor::Module Log("VISIBuilder");
const VISIBuilder::Leaf VISIBuilder::NullLeaf = {};

VISIBuilder::PVSRenderCache::PVSRenderCache(VISIRenderer& renderer)
: m_renderer(renderer)
{
    m_cache.reserve(1000);
}

static std::unique_ptr<VISIRenderer::RGBA8[]> RGBABuf(new VISIRenderer::RGBA8[256 * 256 * 6]);

const VISIBuilder::Leaf& VISIBuilder::PVSRenderCache::GetLeaf(const zeus::CVector3f& vec)
{
    auto search = m_cache.find(vec);
    if (search != m_cache.cend())
    {
        //Log.report(logvisor::Info, "Cache hit");
        return *search->second;
    }

    //Log.report(logvisor::Info, "Rendering");
    bool needsTransparent = false;
    m_renderer.RenderPVSOpaque(RGBABuf.get(), vec, needsTransparent);
    std::unique_ptr<Leaf> leafOut = std::make_unique<Leaf>();
    for (unsigned i=0 ; i<768*512 ; ++i)
    {
        const VISIRenderer::RGBA8& pixel = RGBABuf[i];
        uint32_t id = (pixel.b << 16) | (pixel.g << 8) | pixel.r;
        if (id != 0xffffff)
            leafOut->setBit(id);
    }

    auto setBitLambda = [&](int idx) { leafOut->setBit(idx); };
    auto setLightLambda = [&](int idx, EPVSVisSetState state)
    {
        if (state != EPVSVisSetState::EndOfTree)
            leafOut->setLightEnum(m_lightMetaBit + idx * 2, state);
    };
    if (needsTransparent)
        m_renderer.RenderPVSTransparent(setBitLambda, vec);
    m_renderer.RenderPVSEntitiesAndLights(setBitLambda, setLightLambda, vec);

    return *m_cache.emplace(std::make_pair(vec, std::move(leafOut))).first->second;
}

void VISIBuilder::Progress::report(int divisions)
{
    m_prog += 1.f / divisions;
    printf(" %g%%        \r", m_prog * 100.f);
    fflush(stdout);
    if (m_updatePercent)
        m_updatePercent(m_prog);
}

void VISIBuilder::Node::buildChildren(int level, int divisions, const zeus::CAABox& curAabb,
                                      PVSRenderCache& rc, Progress& prog, const bool& terminate)
{
    if (terminate)
        return;

    // Recurse in while building node structure
    if (level < VISI_MAX_LEVEL)
    {
        // Heuristic split
        int splits[3];
        splits[0] = (curAabb.max.x - curAabb.min.x >= VISI_MIN_LENGTH) ? 2 : 1;
        splits[1] = (curAabb.max.y - curAabb.min.y >= VISI_MIN_LENGTH) ? 2 : 1;
        splits[2] = (curAabb.max.z - curAabb.min.z >= VISI_MIN_LENGTH) ? 2 : 1;

        if (splits[0] == 2)
            flags |= 0x1;
        if (splits[1] == 2)
            flags |= 0x2;
        if (splits[2] == 2)
            flags |= 0x4;

        int thisdiv = splits[0] * splits[1] * splits[2] * divisions;

        if (flags)
        {
            childNodes.resize(8);

            // Inward subdivide
            zeus::CAABox Z[2];
            if (flags & 0x4)
                curAabb.splitZ(Z[0], Z[1]);
            else
                Z[0] = curAabb;
            for (int i=0 ; i<splits[2] ; ++i)
            {
                zeus::CAABox Y[2];
                if (flags & 0x2)
                    Z[i].splitY(Y[0], Y[1]);
                else
                    Y[0] = Z[i];
                for (int j=0 ; j<splits[1] ; ++j)
                {
                    zeus::CAABox X[2];
                    if (flags & 0x1)
                        Y[j].splitX(X[0], X[1]);
                    else
                        X[0] = Y[j];
                    for (int k=0 ; k<splits[0] ; ++k)
                    {
                        childNodes[i*4 + j*2 + k].buildChildren(level + 1, thisdiv, X[k], rc, prog, terminate);
                    }
                }
            }

            // Outward unsubdivide for like-leaves
            for (int i=0 ; i<3 ; ++i)
            {
                if (flags & 0x4 &&
                    childNodes[0] == childNodes[4] &&
                    (!(flags & 0x1) || childNodes[1] == childNodes[5]) &&
                    (!(flags & 0x2) || childNodes[2] == childNodes[6]) &&
                    (!(flags & 0x3) || childNodes[3] == childNodes[7]))
                {
                    flags &= ~0x4;
                    //Log.report(logvisor::Info, "Unsub Z");
                    continue;
                }
                if (flags & 0x2 &&
                    childNodes[0] == childNodes[2] &&
                    (!(flags & 0x1) || childNodes[1] == childNodes[3]) &&
                    (!(flags & 0x4) || childNodes[4] == childNodes[6]) &&
                    (!(flags & 0x5) || childNodes[5] == childNodes[7]))
                {
                    flags &= ~0x2;
                    //Log.report(logvisor::Info, "Unsub Y");
                    continue;
                }
                if (flags & 0x1 &&
                    childNodes[0] == childNodes[1] &&
                    (!(flags & 0x2) || childNodes[2] == childNodes[3]) &&
                    (!(flags & 0x4) || childNodes[4] == childNodes[5]) &&
                    (!(flags & 0x6) || childNodes[6] == childNodes[7]))
                {
                    flags &= ~0x1;
                    //Log.report(logvisor::Info, "Unsub X");
                    continue;
                }
                break;
            }

            if (!flags)
            {
                // This is now a leaf node
                for (int i=0 ; i<8 ; ++i)
                    leaf |= childNodes[i].leaf;
                //Log.report(logvisor::Info, "Leaf Promote");
                return;
            }
        }
    }

    if (!flags)
    {
        // This is a child node
        zeus::CVector3f center = curAabb.center();
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.min.x, curAabb.min.y, curAabb.min.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(center.x, curAabb.min.y, curAabb.min.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.max.x, curAabb.min.y, curAabb.min.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.min.x, center.y, curAabb.min.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(center.x, center.y, curAabb.min.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.max.x, center.y, curAabb.min.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.min.x, curAabb.max.y, curAabb.min.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(center.x, curAabb.max.y, curAabb.min.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.max.x, curAabb.max.y, curAabb.min.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.min.x, curAabb.min.y, center.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(center.x, curAabb.min.y, center.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.max.x, curAabb.min.y, center.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.min.x, center.y, center.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(center.x, center.y, center.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.max.x, center.y, center.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.min.x, curAabb.max.y, center.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(center.x, curAabb.max.y, center.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.max.x, curAabb.max.y, center.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.min.x, curAabb.min.y, curAabb.max.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(center.x, curAabb.min.y, curAabb.max.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.max.x, curAabb.min.y, curAabb.max.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.min.x, center.y, curAabb.max.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(center.x, center.y, curAabb.max.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.max.x, center.y, curAabb.max.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.min.x, curAabb.max.y, curAabb.max.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(center.x, curAabb.max.y, curAabb.max.z));
        leaf |= rc.GetLeaf(zeus::CVector3f(curAabb.max.x, curAabb.max.y, curAabb.max.z));

        prog.report(divisions);
    }
}

static const int NumChildTable[] =
{
    0, 2, 2, 4, 2, 4, 4, 8
};

void VISIBuilder::Node::calculateSizesAndOffs(size_t& cur, size_t leafSz)
{
    cur += 1;

    if (flags)
    {
        int splits[3];
        splits[0] = (flags & 0x1) ? 2 : 1;
        splits[1] = (flags & 0x2) ? 2 : 1;
        splits[2] = (flags & 0x4) ? 2 : 1;

        // Inward accumulate
        const size_t startCur = cur;
        size_t maxDelta = 0;
        for (int i=0 ; i<splits[2] ; ++i)
            for (int j=0 ; j<splits[1] ; ++j)
                for (int k=0 ; k<splits[0] ; ++k)
                {
                    const size_t nodeSel = i*4 + j*2 + k;
                    const size_t delta = cur - startCur;
                    if (delta > maxDelta)
                        maxDelta = delta;
                    childRelOffs[nodeSel] = delta;
                    childNodes[nodeSel].calculateSizesAndOffs(cur, leafSz);
                }

        const int numChildren = NumChildTable[flags & 0x7];
        if (maxDelta > 0xffff)
        {
            cur += (numChildren - 1) * 3;
            flags |= 0x40;
        }
        else if (maxDelta > 0xff)
        {
            cur += (numChildren - 1) * 2;
        }
        else
        {
            cur += numChildren - 1;
            flags |= 0x20;
        }
    }
    else
    {
        cur += leafSz;
        flags |= 0x18;
    }
}

void VISIBuilder::Node::writeNodes(athena::io::MemoryWriter& w, size_t leafBytes) const
{
    w.writeUByte(flags);

    if (flags & 0x7)
    {
        int splits[3];
        splits[0] = (flags & 0x1) ? 2 : 1;
        splits[1] = (flags & 0x2) ? 2 : 1;
        splits[2] = (flags & 0x4) ? 2 : 1;

        // Write offsets
        for (int i=0 ; i<splits[2] ; ++i)
            for (int j=0 ; j<splits[1] ; ++j)
                for (int k=0 ; k<splits[0] ; ++k)
                {
                    const size_t nodeSel = i*4 + j*2 + k;
                    if (nodeSel == 0)
                        continue;
                    const size_t offset = childRelOffs[nodeSel];
                    if (flags & 0x40)
                    {
                        w.writeUByte((offset >> 16) & 0xff);
                        w.writeUByte((offset >> 8) & 0xff);
                        w.writeUByte(offset & 0xff);
                    }
                    else if (flags & 0x20)
                    {
                        w.writeUByte(offset & 0xff);
                    }
                    else
                    {
                        w.writeUint16Big(offset);
                    }
                }

        // Inward iterate
        for (int i=0 ; i<splits[2] ; ++i)
            for (int j=0 ; j<splits[1] ; ++j)
                for (int k=0 ; k<splits[0] ; ++k)
                {
                    const size_t nodeSel = i*4 + j*2 + k;
                    childNodes[nodeSel].writeNodes(w, leafBytes);
                }
    }
    else
    {
        leaf.write(w, leafBytes);
    }
}

std::vector<uint8_t> VISIBuilder::build(const zeus::CAABox& fullAabb,
                                        size_t modelCount,
                                        const std::vector<VISIRenderer::Entity>& entities,
                                        const std::vector<VISIRenderer::Light>& lights,
                                        FPercent updatePercent)
{
    Log.report(logvisor::Info, "Started!");

    size_t featureCount = modelCount + entities.size();
    renderCache.m_lightMetaBit = featureCount + lights.size();

    Progress prog(updatePercent);
    bool& terminate = renderCache.m_renderer.m_terminate;
    rootNode.buildChildren(0, 1, fullAabb, renderCache, prog, terminate);
    if (terminate)
        return {};

    // Lights cache their CPVSVisSet result enum as 2 bits
    size_t leafBitsCount = featureCount + lights.size() * 3;
    size_t leafBytesCount = ROUND_UP_8(leafBitsCount) / 8;

    // Calculate octree size and store relative offsets
    size_t octreeSz = 0;
    rootNode.calculateSizesAndOffs(octreeSz, leafBytesCount);
    octreeSz += 1; // Terminator node

    size_t visiSz = 34 + entities.size() * 4 + lights.size() * leafBytesCount + 36 + octreeSz;
    size_t roundedVisiSz = ROUND_UP_32(visiSz);

    std::vector<uint8_t> dataOut(roundedVisiSz, 0);
    athena::io::MemoryWriter w(dataOut.data(), roundedVisiSz);
    w.writeUint32Big('VISI');
    w.writeUint32Big(2);
    w.writeBool(true);
    w.writeBool(true);
    w.writeUint32Big(featureCount);
    w.writeUint32Big(lights.size());
    w.writeUint32Big(0);
    w.writeUint32Big(entities.size());
    w.writeUint32Big(leafBytesCount);
    w.writeUint32Big(lights.size());

    for (const VISIRenderer::Entity& e : entities)
    {
        w.writeUint32Big(e.entityId);
    }

    for (const VISIRenderer::Light& l : lights)
    {
        const VISIBuilder::Leaf& leaf = renderCache.GetLeaf(l.point);
        leaf.write(w, leafBytesCount);
    }

    w.writeVec3fBig(fullAabb.min);
    w.writeVec3fBig(fullAabb.max);
    w.writeUint32Big(leafBitsCount);
    w.writeUint32Big(lights.size());
    w.writeUint32Big(octreeSz);
    rootNode.writeNodes(w, leafBytesCount);
    w.writeUByte(0x10);

    w.seekAlign32();

    printf("\n");
    Log.report(logvisor::Info, "Finished!");
    return dataOut;
}
