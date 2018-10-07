#ifndef HECLRUNTIME_HPP
#define HECLRUNTIME_HPP

#include "hecl.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include <unordered_map>

namespace hecl
{
struct HMDLMeta;

namespace Runtime
{

/**
 * @brief Per-platform file store resolution
 */
class FileStoreManager
{
    SystemString m_domain;
    SystemString m_storeRoot;
public:
    FileStoreManager(SystemStringView domain);
    SystemStringView getDomain() const {return m_domain;}
    /**
     * @brief Returns the full path to the file store, including domain
     * @return Full path to store e.g /home/foo/.hecl/bar
     */
    SystemStringView getStoreRoot() const {return m_storeRoot;}
};

/**
 * @brief Integrated reader/constructor/container for HMDL data
 */
struct HMDLData
{
    boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferS> m_ibo;
    std::unique_ptr<boo::VertexElementDescriptor[]> m_vtxFmtData;
    boo::VertexFormatInfo m_vtxFmt;

    HMDLData(boo::IGraphicsDataFactory::Context& ctx,
             const void* metaData, const void* vbo, const void* ibo);

    boo::ObjToken<boo::IShaderDataBinding>
    newShaderDataBindng(boo::IGraphicsDataFactory::Context& ctx,
                        const boo::ObjToken<boo::IShaderPipeline>& shader,
                        size_t ubufCount, const boo::ObjToken<boo::IGraphicsBuffer>* ubufs,
                        const boo::PipelineStage* ubufStages,
                        size_t texCount, const boo::ObjToken<boo::ITexture>* texs)
    {return ctx.newShaderDataBinding(shader, m_vbo.get(), nullptr, m_ibo.get(),
                                     ubufCount, ubufs, ubufStages, nullptr, nullptr,
                                     texCount, texs, nullptr, nullptr);}
};

}
}

#endif // HECLRUNTIME_HPP
