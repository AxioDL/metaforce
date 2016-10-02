#ifndef SPECBASE_HPP
#define SPECBASE_HPP

#include <functional>

#include <hecl/Database.hpp>
#include <nod/nod.hpp>
#include "hecl/Blender/BlenderConnection.hpp"

namespace urde
{
struct SObjectTag;
}

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

    bool canPackage(const PackagePassInfo& info);
    void doPackage(const PackagePassInfo& info);

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
    virtual urde::SObjectTag BuildTagFromPath(const hecl::ProjectPath& path,
                                              hecl::BlenderToken& btok) const=0;

    /* Even if PC spec is being cooked, this will return the vanilla GCN spec */
    virtual const hecl::Database::DataSpecEntry* getOriginalSpec() const=0;

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
    virtual void cookActor(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                           BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                           FCookProgress progress)=0;
    virtual void cookArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                          BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                          FCookProgress progress)=0;
    virtual void cookWorld(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                           BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                           FCookProgress progress)=0;
    virtual void cookYAML(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                          athena::io::IStreamReader& fin, FCookProgress progress)=0;
    virtual void cookAudioGroup(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                                FCookProgress progress)=0;
    virtual void cookSong(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                          FCookProgress progress)=0;

    /* Dependency flatteners */
    void flattenDependencies(const hecl::ProjectPath& in,
                             std::vector<hecl::ProjectPath>& pathsOut,
                             hecl::BlenderToken& btok);
    void flattenDependencies(const class UniqueID32& id, std::vector<hecl::ProjectPath>& pathsOut);
    void flattenDependencies(const class UniqueID64& id, std::vector<hecl::ProjectPath>& pathsOut);
    virtual void flattenDependenciesYAML(athena::io::IStreamReader& fin, std::vector<hecl::ProjectPath>& pathsOut)=0;

    const hecl::ProjectPath& getMasterShaderPath() const {return m_masterShader;}

    /* Support functions for resolving paths from IDs */
    virtual hecl::ProjectPath getWorking(class UniqueID32&) {return hecl::ProjectPath();}
    virtual hecl::ProjectPath getWorking(class UniqueID64&) {return hecl::ProjectPath();}

    /* Project accessor */
    hecl::Database::Project& getProject() const {return m_project;}

    SpecBase(const hecl::Database::DataSpecEntry* specEntry, hecl::Database::Project& project, bool pc);
protected:
    hecl::Database::Project& m_project;
    bool m_pc;
    hecl::ProjectPath m_masterShader;
private:
    std::unique_ptr<nod::DiscBase> m_disc;
    bool m_isWii;
    bool m_standalone;
};

}

#endif // SPECBASE_HPP

