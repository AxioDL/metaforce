#include "HECL/Runtime.hpp"
#include <Athena/FileReader.hpp>
#include <Athena/FileWriter.hpp>
#include <zlib.h>
#include <algorithm>
#include <ctime>

#include "HECL/Backend/GLSL.hpp"
#if __APPLE__
#include "HECL/Backend/Metal.hpp"
#endif

namespace HECL
{
namespace Runtime
{
IShaderBackendFactory* _NewGLSLBackendFactory(boo::IGraphicsDataFactory* gfxFactory);
#if _WIN32
IShaderBackendFactory* _NewHLSLBackendFactory(boo::IGraphicsDataFactory* gfxFactory);
#elif __APPLE__
IShaderBackendFactory* _NewMetalBackendFactory(boo::IGraphicsDataFactory* gfxFactory);
#endif

static LogVisor::LogModule Log("ShaderCacheManager");
static uint64_t IDX_MAGIC = SBig(uint64_t(0xDEADFEEDC001D00D));
static uint64_t DAT_MAGIC = SBig(uint64_t(0xC001D00DDEADBABE));
static uint64_t ZERO64 = 0;

static uint64_t timeHash()
{
    char buf[80];
    time_t now;
    struct tm* timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    strftime(buf, 80, "%Y-%m-%dT%H:%M:%S+%H:%M", timeinfo);
    Hash tmp(buf, 80);
    return tmp.val64();
}

static void UpdateFunctionHash(XXH64_state_t* st, const ShaderCacheExtensions::Function& func)
{
    if (func.m_source)
        XXH64_update(st, func.m_source, strlen(func.m_source));
    if (func.m_entry)
        XXH64_update(st, func.m_entry, strlen(func.m_entry));
}
uint64_t ShaderCacheExtensions::hashExtensions() const
{
    XXH64_state_t st;
    XXH64_reset(&st, 0);
    for (const ExtensionSlot& slot : m_extensionSlots)
    {
        UpdateFunctionHash(&st, slot.lighting);
        UpdateFunctionHash(&st, slot.post);
    }
    return XXH64_digest(&st);
}

void ShaderCacheManager::bootstrapIndex()
{
    m_timeHash = timeHash();
    m_idxFr.close();
    m_datFr.close();

#if _WIN32
    SystemString idxFilename = m_idxFr.wfilename();
#else
    SystemString idxFilename = m_idxFr.filename();
#endif

    FILE* idxFp = HECL::Fopen(idxFilename.c_str(), _S("wb"));
    if (!idxFp)
        Log.report(LogVisor::FatalError, _S("unable to write shader cache index at %s"),
                   idxFilename.c_str());
    fwrite(&IDX_MAGIC, 1, 8, idxFp);
    fwrite(&m_timeHash, 1, 8, idxFp);
    fwrite(&m_extensionsHash, 1, 8, idxFp);
    fwrite(&ZERO64, 1, 8, idxFp);
    fclose(idxFp);
#if _WIN32
    SystemString datFilename = m_datFr.wfilename();
#else
    SystemString datFilename = m_datFr.filename();
#endif

    FILE* datFp = HECL::Fopen(datFilename.c_str(), _S("wb"));
    if (!datFp)
        Log.report(LogVisor::FatalError, _S("unable to write shader cache data at %s"),
                   datFilename.c_str());
    fwrite(&DAT_MAGIC, 1, 8, datFp);
    fwrite(&m_timeHash, 1, 8, datFp);
    fclose(datFp);

    m_idxFr.open();
    m_datFr.open();
}

ShaderCacheManager::ShaderCacheManager(const FileStoreManager& storeMgr,
                                       boo::IGraphicsDataFactory* gfxFactory,
                                       ShaderCacheExtensions&& extension)
: m_storeMgr(storeMgr),
  m_extensions(std::move(extension)),
  m_idxFr(storeMgr.getStoreRoot() + _S("/shadercache") + gfxFactory->platformName() + _S(".idx"), 32*1024, false),
  m_datFr(storeMgr.getStoreRoot() + _S("/shadercache") + gfxFactory->platformName() + _S(".dat"), 32*1024, false)
{
    boo::IGraphicsDataFactory::Platform plat = gfxFactory->platform();
    if (m_extensions && m_extensions.m_plat != plat)
        Log.report(LogVisor::FatalError, "ShaderCacheExtension backend mismatch (should be %s)",
                   gfxFactory->platformName());
    m_extensionsHash = m_extensions.hashExtensions();

    switch (plat)
    {
    case boo::IGraphicsDataFactory::Platform::OGL:
        m_factory.reset(_NewGLSLBackendFactory(gfxFactory));
        break;
#if _WIN32
    case boo::IGraphicsDataFactory::Platform::D3D11:
    case boo::IGraphicsDataFactory::Platform::D3D12:
        m_factory.reset(_NewHLSLBackendFactory(gfxFactory));
        break;
#elif __APPLE__ && HECL_HAS_METAL
    case boo::IGraphicsDataFactory::Platform::Metal:
        m_factory.reset(_NewMetalBackendFactory(gfxFactory));
        break;
#endif
    default:
        Log.report(LogVisor::FatalError, _S("unsupported backend %s"), gfxFactory->platformName());
    }

    reload();
}

void ShaderCacheManager::reload()
{
    m_entries.clear();
    m_entryLookup.clear();
    m_timeHash = 0;

    /* Attempt to open existing index */
    m_idxFr.seek(0, Athena::Begin);
    m_datFr.seek(0, Athena::Begin);
    if (m_idxFr.hasError() || m_datFr.hasError())
    {
        bootstrapIndex();
        return;
    }
    else
    {
        uint64_t idxMagic;
        size_t rb = m_idxFr.readUBytesToBuf(&idxMagic, 8);
        if (rb != 8 || idxMagic != IDX_MAGIC)
        {
            bootstrapIndex();
            return;
        }

        uint64_t datMagic;
        rb = m_datFr.readUBytesToBuf(&datMagic, 8);
        if (rb != 8 || datMagic != DAT_MAGIC)
        {
            bootstrapIndex();
            return;
        }

        uint64_t idxRand, datRand;
        rb = m_idxFr.readUBytesToBuf(&idxRand, 8);
        size_t rb2 = m_datFr.readUBytesToBuf(&datRand, 8);
        if (rb != 8 || rb2 != 8 || idxRand != datRand)
        {
            bootstrapIndex();
            return;
        }
        m_timeHash = idxRand;
    }

    atUint64 extensionsHash;
    size_t rb = m_idxFr.readUBytesToBuf(&extensionsHash, 8);
    if (rb != 8 || extensionsHash != m_extensionsHash)
    {
        bootstrapIndex();
        return;
    }

    atUint64 idxCount = m_idxFr.readUint64Big();
    if (m_idxFr.position() != 32)
    {
        bootstrapIndex();
        return;
    }

    /* Read existing entries */
    if (idxCount)
    {
        m_entries.reserve(idxCount);
        m_entryLookup.reserve(idxCount);
        for (atUint64 i=0 ; i<idxCount ; ++i)
        {
            m_entries.emplace_back();
            IndexEntry& ent = m_entries.back();
            ent.read(m_idxFr);
            m_entryLookup[ent.m_hash] = m_entries.size() - 1;
        }
    }
}

ShaderCachedData ShaderCacheManager::lookupData(const Hash& hash)
{
    auto search = m_entryLookup.find(hash);
    if (search == m_entryLookup.cend())
        return ShaderCachedData();

    const IndexEntry& ent = m_entries[search->second];
    if (ent.m_compOffset + ent.m_compSize > m_datFr.length())
    {
        Log.report(LogVisor::Warning, "shader cache not long enough to read entry, might be corrupt");
        return ShaderCachedData();
    }

    /* File-streamed decompression */
    m_datFr.seek(ent.m_compOffset, Athena::Begin);
    ShaderCachedData ret(ShaderTag(ent.m_hash, ent.m_meta), ent.m_decompSize);
    uint8_t compDat[2048];
    z_stream z = {};
    inflateInit(&z);
    z.avail_out = ent.m_decompSize;
    z.next_out = ret.m_data.get();
    while (z.avail_out)
    {
        z.avail_in = std::min(size_t(2048), size_t(ent.m_compSize - z.total_in));
        m_datFr.readUBytesToBuf(compDat, z.avail_in);
        z.next_in = compDat;
        inflate(&z, Z_NO_FLUSH);
    }
    inflateEnd(&z);
    return ret;
}

bool ShaderCacheManager::addData(const ShaderCachedData& data)
{
    m_idxFr.close();
    m_datFr.close();

    /* Perform one-shot buffer compression */
    uLong cBound = compressBound(data.m_sz);
    void* compBuf = malloc(cBound);
    if (compress((Bytef*)compBuf, &cBound, (Bytef*)data.m_data.get(), data.m_sz) != Z_OK)
        Log.report(LogVisor::FatalError, "unable to deflate data");

    /* Open index for writing (non overwriting) */
    Athena::io::FileWriter idxFw(m_idxFr.filename(), false);
    if (idxFw.hasError())
        Log.report(LogVisor::FatalError, _S("unable to append shader cache index at %s"),
                   m_idxFr.filename().c_str());

    /* Open data for writing (non overwriting) */
    Athena::io::FileWriter datFw(m_datFr.filename(), false);
    if (datFw.hasError())
        Log.report(LogVisor::FatalError, _S("unable to append shader cache data at %s"),
                   m_datFr.filename().c_str());

    size_t targetOffset = 0;
    auto search = m_entryLookup.find(data.m_tag);
    if (search != m_entryLookup.cend())
    {
        /* Hash already present, attempt to replace data */
        IndexEntry& ent = m_entries[search->second];
        if (search->second == m_entries.size() - 1)
        {
            /* Replacing final entry; simply write-over */
            ent.m_meta = data.m_tag.getMetaData();
            ent.m_compSize = cBound;
            ent.m_decompSize = data.m_sz;
            targetOffset = ent.m_compOffset;
            idxFw.seek(search->second * 32 + 32);
            ent.write(idxFw);
        }
        else
        {
            /* Replacing non-final entry; write into available space */
            IndexEntry& nent = m_entries[search->second+1];
            size_t space = nent.m_compOffset - ent.m_compOffset;
            if (cBound <= space)
            {
                ent.m_meta = data.m_tag.getMetaData();
                ent.m_compSize = cBound;
                ent.m_decompSize = data.m_sz;
                targetOffset = ent.m_compOffset;
                idxFw.seek(search->second * 32 + 32);
                ent.write(idxFw);
            }
            else
            {
                /* Not enough space; null-entry and add to end */
                ent.m_hash = 0;
                ent.m_meta = 0;
                ent.m_compOffset = 0;
                ent.m_compSize = 0;
                ent.m_decompSize = 0;
                idxFw.seek(search->second * 32 + 32);
                ent.write(idxFw);
            }
        }
    }

    if (!targetOffset)
    {
        /* New index entry at end */
        idxFw.seek(24, Athena::Begin);
        idxFw.writeUint64Big(m_entries.size() + 1);
        idxFw.seek(m_entries.size() * 32 + 32, Athena::Begin);
        datFw.seek(0, Athena::End);
        m_entryLookup[data.m_tag] = m_entries.size();
        m_entries.emplace_back();

        IndexEntry& ent = m_entries.back();
        ent.m_hash = data.m_tag.val64();
        ent.m_meta = data.m_tag.getMetaData();
        ent.m_compOffset = datFw.position();
        ent.m_compSize = cBound;
        ent.m_decompSize = data.m_sz;
        ent.write(idxFw);

        datFw.writeUBytes((atUint8*)compBuf, cBound);
    }
    else
    {
        /* Reusing index entry and data space */
        datFw.seek(targetOffset, Athena::Begin);
        datFw.writeUBytes((atUint8*)compBuf, cBound);
    }

    free(compBuf);

    idxFw.close();
    datFw.close();

    m_idxFr.open();
    m_datFr.open();

    return true;
}

boo::IShaderPipeline*
ShaderCacheManager::buildFromCache(const ShaderCachedData& foundData)
{
    return m_factory->buildShaderFromCache(foundData);
}

boo::IShaderPipeline*
ShaderCacheManager::buildShader(const ShaderTag& tag, const std::string& source,
                                const std::string& diagName)
{
    ShaderCachedData foundData = lookupData(tag);
    if (foundData)
        return buildFromCache(foundData);
    HECL::Frontend::IR ir = FE.compileSource(source, diagName);
    return buildShader(tag, ir, diagName);
}

boo::IShaderPipeline*
ShaderCacheManager::buildShader(const ShaderTag& tag, const HECL::Frontend::IR& ir,
                                const std::string& diagName)
{
    ShaderCachedData foundData = lookupData(tag);
    if (foundData)
        return buildFromCache(foundData);
    FE.getDiagnostics().reset(diagName);
    boo::IShaderPipeline* ret;
    addData(m_factory->buildShaderFromIR(tag, ir, FE.getDiagnostics(), &ret));
    return ret;
}

std::vector<boo::IShaderPipeline*>
ShaderCacheManager::buildExtendedFromCache(const ShaderCachedData& foundData)
{
    std::vector<boo::IShaderPipeline*> shaders;
    shaders.reserve(m_extensions.m_extensionSlots.size());
    m_factory->buildExtendedShaderFromCache(foundData, m_extensions.m_extensionSlots,
    [&](boo::IShaderPipeline* shader){shaders.push_back(shader);});
    if (shaders.size() != m_extensions.m_extensionSlots.size())
        Log.report(LogVisor::FatalError, "buildShaderFromCache returned %" PRISize " times, expected %" PRISize,
                   shaders.size(), m_extensions.m_extensionSlots.size());
    return shaders;
}

std::vector<boo::IShaderPipeline*>
ShaderCacheManager::buildExtendedShader(const ShaderTag& tag, const std::string& source,
                                        const std::string& diagName)
{
    ShaderCachedData foundData = lookupData(tag);
    if (foundData)
        return buildExtendedFromCache(foundData);
    HECL::Frontend::IR ir = FE.compileSource(source, diagName);
    return buildExtendedShader(tag, ir, diagName);
}

std::vector<boo::IShaderPipeline*>
ShaderCacheManager::buildExtendedShader(const ShaderTag& tag, const HECL::Frontend::IR& ir,
                                        const std::string& diagName)
{
    ShaderCachedData foundData = lookupData(tag);
    if (foundData)
        return buildExtendedFromCache(foundData);
    std::vector<boo::IShaderPipeline*> shaders;
    shaders.reserve(m_extensions.m_extensionSlots.size());
    FE.getDiagnostics().reset(diagName);
    ShaderCachedData data =
    m_factory->buildExtendedShaderFromIR(tag, ir, FE.getDiagnostics(), m_extensions.m_extensionSlots,
    [&](boo::IShaderPipeline* shader){shaders.push_back(shader);});
    if (shaders.size() != m_extensions.m_extensionSlots.size())
        Log.report(LogVisor::FatalError, "buildShaderFromIR returned %" PRISize " times, expected %" PRISize,
                   shaders.size(), m_extensions.m_extensionSlots.size());
    addData(data);
    return shaders;
}

}
}
