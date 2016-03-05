#ifndef SPECBASE_HPP
#define SPECBASE_HPP

#include <functional>

#include <hecl/Database.hpp>
#include <nod/nod.hpp>
#include "BlenderConnection.hpp"

namespace DataSpec
{

struct SpecBase : hecl::Database::IDataSpec
{
    /* HECL Adaptors */
    bool canExtract(const ExtractPassInfo& info, std::vector<ExtractReport>& reps);
    void doExtract(const ExtractPassInfo& info, FProgress progress);

    bool canCook(const hecl::ProjectPath& path);
    void doCook(const hecl::ProjectPath& path, const hecl::ProjectPath& cookedPath, bool fast, FCookProgress progress);

    bool canPackage(const PackagePassInfo& info);
    void gatherDependencies(const PackagePassInfo& info,
                            std::unordered_set<hecl::ProjectPath>& implicitsOut);
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

    /* Basic path check (game directory matching) */
    virtual bool checkPathPrefix(const hecl::ProjectPath& path)=0;

    /* Pre-cook handlers */
    virtual bool validateYAMLDNAType(FILE* fp) const=0;

    /* Cook handlers */
    using BlendStream = hecl::BlenderConnection::DataStream;
    using Mesh = BlendStream::Mesh;
    using Actor = BlendStream::Actor;

    virtual void cookMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast, FCookProgress progress) const=0;
    virtual void cookActor(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast, FCookProgress progress) const=0;
    virtual void cookArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast, FCookProgress progress) const=0;
    virtual void cookYAML(const hecl::ProjectPath& out, const hecl::ProjectPath& in, FILE* fin, FCookProgress progress) const=0;

    const hecl::ProjectPath& getMasterShaderPath() const {return m_masterShader;}

    /* Support functions for resolving paths from IDs */
    virtual hecl::ProjectPath getWorking(class UniqueID32&) {return hecl::ProjectPath();}
    virtual hecl::ProjectPath getWorking(class UniqueID64&) {return hecl::ProjectPath();}

    /* Project accessor */
    hecl::Database::Project& getProject() const {return m_project;}

    SpecBase(hecl::Database::Project& project);
protected:
    hecl::Database::Project& m_project;
    hecl::ProjectPath m_masterShader;
private:
    std::unique_ptr<nod::DiscBase> m_disc;
    bool m_isWii;
    bool m_standalone;
};

}

#endif // SPECBASE_HPP

