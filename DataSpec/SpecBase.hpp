#ifndef SPECBASE_HPP
#define SPECBASE_HPP

#include <functional>
#include <thread>

#include <hecl/Database.hpp>
#include <nod/nod.hpp>
#include <athena/DNAYaml.hpp>
#include <athena/FileWriter.hpp>
#include "hecl/Blender/BlenderConnection.hpp"
#include "Runtime/RetroTypes.hpp"

namespace DataSpec
{

struct SpecBase : hecl::Database::IDataSpec
{
    /* HECL Adaptors */
    void setThreadProject();
    bool canExtract(const ExtractPassInfo& info, std::vector<ExtractReport>& reps);
    void doExtract(const ExtractPassInfo& info, FProgress progress);

    bool canCook(const hecl::ProjectPath& path, hecl::BlenderToken& btok);
    const hecl::Database::DataSpecEntry* overrideDataSpec(const hecl::ProjectPath& path,
                                                          const hecl::Database::DataSpecEntry* oldEntry,
                                                          hecl::BlenderToken& btok) const;
    void doCook(const hecl::ProjectPath& path, const hecl::ProjectPath& cookedPath,
                bool fast, hecl::BlenderToken& btok, FCookProgress progress);

    bool canPackage(const hecl::ProjectPath& path);
    void doPackage(const hecl::ProjectPath& path, const hecl::Database::DataSpecEntry* entry,
                   bool fast, hecl::BlenderToken& btok, FProgress progress, hecl::ClientProcess* cp);

    /* Extract handlers */
    virtual bool checkStandaloneID(const char* id) const=0;
    virtual bool checkFromStandaloneDisc(nod::DiscBase& disc,
                                         const hecl::SystemString& regstr,
                                         const std::vector<hecl::SystemString>& args,
                                         std::vector<ExtractReport>& reps)=0;
    virtual bool checkFromTrilogyDisc(nod::DiscBase& disc,
                                      const hecl::SystemString& regstr,
                                      const std::vector<hecl::SystemString>& args,
                                      std::vector<ExtractReport>& reps)=0;
    virtual bool extractFromDisc(nod::DiscBase& disc, bool force,
                                 FProgress progress)=0;

    /* Convert path to object tag */
    virtual urde::SObjectTag buildTagFromPath(const hecl::ProjectPath& path,
                                              hecl::BlenderToken& btok) const=0;

    /* Even if PC spec is being cooked, this will return the vanilla GCN spec */
    virtual const hecl::Database::DataSpecEntry& getOriginalSpec() const=0;

    /* This will return a pseudo-spec for unmodified resources */
    virtual const hecl::Database::DataSpecEntry& getUnmodifiedSpec() const=0;

    /* Basic path check (game directory matching) */
    virtual bool checkPathPrefix(const hecl::ProjectPath& path) const=0;

    /* Pre-cook handlers */
    virtual bool validateYAMLDNAType(athena::io::IStreamReader& fp) const=0;

    /* Cook handlers */
    using BlendStream = hecl::BlenderConnection::DataStream;
    using Mesh = BlendStream::Mesh;
    using ColMesh = BlendStream::ColMesh;
    using Light = BlendStream::Light;
    using Actor = BlendStream::Actor;

    virtual void cookMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                          BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                          FCookProgress progress)=0;
    virtual void cookColMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                             BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                             FCookProgress progress)=0;
    virtual void cookActor(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                           BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                           FCookProgress progress)=0;
    virtual void cookArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                          BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                          FCookProgress progress)=0;
    virtual void cookWorld(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                           BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                           FCookProgress progress)=0;
    virtual void cookGuiFrame(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                              BlendStream& ds, hecl::BlenderToken& btok,
                              FCookProgress progress)=0;
    virtual void cookYAML(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                          athena::io::IStreamReader& fin, FCookProgress progress)=0;
    virtual void cookAudioGroup(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                                FCookProgress progress)=0;
    virtual void cookSong(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                          FCookProgress progress)=0;
    virtual void cookMapArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                             BlendStream& ds, hecl::BlenderToken& btok,
                             FCookProgress progress)=0;
    virtual void cookMapUniverse(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                                 BlendStream& ds, hecl::BlenderToken& btok,
                                 FCookProgress progress)=0;

    /* Dependency flatteners */
    void flattenDependencies(const hecl::ProjectPath& in,
                             std::vector<hecl::ProjectPath>& pathsOut,
                             hecl::BlenderToken& btok);
    void flattenDependencies(const class UniqueID32& id, std::vector<hecl::ProjectPath>& pathsOut);
    void flattenDependencies(const class UniqueID64& id, std::vector<hecl::ProjectPath>& pathsOut);
    virtual void flattenDependenciesYAML(athena::io::IStreamReader& fin, std::vector<hecl::ProjectPath>& pathsOut)=0;

    virtual void buildWorldPakList(const hecl::ProjectPath& worldPath,
                                   const hecl::ProjectPath& worldPathCooked,
                                   hecl::BlenderToken& btok,
                                   athena::io::FileWriter& w,
                                   std::vector<urde::SObjectTag>& listOut,
                                   atUint64& resTableOffset) {}
    virtual void buildPakList(hecl::BlenderToken& btok,
                              athena::io::FileWriter& w,
                              const std::vector<urde::SObjectTag>& list,
                              const std::vector<std::pair<urde::SObjectTag, std::string>>& nameList,
                              atUint64& resTableOffset) {}
    virtual void writePakFileIndex(athena::io::FileWriter& w,
                                   const std::vector<urde::SObjectTag>& tags,
                                   const std::vector<std::tuple<size_t, size_t, bool>>& index,
                                   atUint64 resTableOffset) {}
    virtual std::pair<std::unique_ptr<uint8_t[]>, size_t>
    compressPakData(const urde::SObjectTag& tag, const uint8_t* data, size_t len) { return {}; }

    const hecl::ProjectPath& getMasterShaderPath() const {return m_masterShader;}

    /* Support functions for resolving paths from IDs */
    virtual hecl::ProjectPath getWorking(class UniqueID32&) {return hecl::ProjectPath();}
    virtual hecl::ProjectPath getWorking(class UniqueID64&) {return hecl::ProjectPath();}

    hecl::ProjectPath getCookedPath(const hecl::ProjectPath& working, bool pcTarget) const;

    /* Project accessor */
    hecl::Database::Project& getProject() const {return m_project;}

    /* Extract RandomStatic entropy */
    void extractRandomStaticEntropy(const uint8_t* buf, const hecl::ProjectPath& noAramPath);

    /* Tag cache functions */
    urde::SObjectTag tagFromPath(const hecl::ProjectPath& path, hecl::BlenderToken& btok) const;
    hecl::ProjectPath pathFromTag(const urde::SObjectTag& tag) const;
    bool waitForTagReady(const urde::SObjectTag& tag, const hecl::ProjectPath*& pathOut);
    const urde::SObjectTag* getResourceIdByName(const char* name) const;
    hecl::FourCC getResourceTypeById(urde::CAssetId id) const;
    void enumerateResources(const std::function<bool(const urde::SObjectTag&)>& lambda) const;
    void enumerateNamedResources(
        const std::function<bool(const std::string&, const urde::SObjectTag&)>& lambda) const;
    void cancelBackgroundIndex();
    void beginBackgroundIndex();
    bool backgroundIndexRunning() const { return m_backgroundRunning; }
    void waitForIndexComplete() const;

    virtual void getTagListForFile(const char* pakName, std::vector<urde::SObjectTag>& out) const {}

    SpecBase(const hecl::Database::DataSpecEntry* specEntry, hecl::Database::Project& project, bool pc);
    ~SpecBase();
protected:
    hecl::Database::Project& m_project;
    bool m_pc;
    hecl::ProjectPath m_masterShader;

    std::unordered_map<urde::SObjectTag, hecl::ProjectPath> m_tagToPath;
    std::unordered_map<hecl::Hash, urde::SObjectTag> m_pathToTag;
    std::unordered_map<std::string, urde::SObjectTag> m_catalogNameToTag;
    std::unordered_map<urde::SObjectTag, std::string> m_catalogTagToName;
    void clearTagCache();

    hecl::BlenderToken m_backgroundBlender;
    std::thread m_backgroundIndexTh;
    std::mutex m_backgroundIndexMutex;
    bool m_backgroundRunning = false;

    void readCatalog(const hecl::ProjectPath& catalogPath,
                     athena::io::YAMLDocWriter& nameWriter);
    bool addFileToIndex(const hecl::ProjectPath& path,
                        athena::io::YAMLDocWriter& cacheWriter);
    void backgroundIndexRecursiveProc(const hecl::ProjectPath& path,
                                      athena::io::YAMLDocWriter& cacheWriter,
                                      athena::io::YAMLDocWriter& nameWriter,
                                      int level);
    void backgroundIndexRecursiveCatalogs(const hecl::ProjectPath& path,
                                          athena::io::YAMLDocWriter& nameWriter,
                                          int level);
    void backgroundIndexProc();

    void recursiveBuildResourceList(std::vector<urde::SObjectTag>& listOut,
                                    const hecl::ProjectPath& path,
                                    hecl::BlenderToken& btok);
    void copyBuildListData(std::vector<std::tuple<size_t, size_t, bool>>& fileIndex,
                           const std::vector<urde::SObjectTag>& buildList,
                           const hecl::Database::DataSpecEntry* entry,
                           bool fast, FProgress progress, athena::io::FileWriter& pakOut);

private:
    std::unique_ptr<nod::DiscBase> m_disc;
    bool m_isWii;
    bool m_standalone;
};

}

#endif // SPECBASE_HPP

