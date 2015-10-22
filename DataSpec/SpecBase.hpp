#ifndef SPECBASE_HPP
#define SPECBASE_HPP

#include <functional>

#include <HECL/Database.hpp>
#include <NOD/NOD.hpp>
#include "BlenderConnection.hpp"

namespace Retro
{

struct SpecBase : HECL::Database::IDataSpec
{
    /* HECL Adaptors */
    bool canExtract(const ExtractPassInfo& info, std::vector<ExtractReport>& reps);
    void doExtract(const ExtractPassInfo& info, FProgress progress);

    bool canCook(const HECL::ProjectPath& path);
    void doCook(const HECL::ProjectPath& path, const HECL::ProjectPath& cookedPath, bool fast, FCookProgress progress);

    bool canPackage(const PackagePassInfo& info);
    void gatherDependencies(const PackagePassInfo& info,
                            std::unordered_set<HECL::ProjectPath>& implicitsOut);
    void doPackage(const PackagePassInfo& info);

    /* Extract handlers */
    virtual bool checkStandaloneID(const char* id) const=0;
    virtual bool checkFromStandaloneDisc(NOD::DiscBase& disc,
                                         const HECL::SystemString& regstr,
                                         const std::vector<HECL::SystemString>& args,
                                         std::vector<ExtractReport>& reps)=0;
    virtual bool checkFromTrilogyDisc(NOD::DiscBase& disc,
                                      const HECL::SystemString& regstr,
                                      const std::vector<HECL::SystemString>& args,
                                      std::vector<ExtractReport>& reps)=0;
    virtual bool extractFromDisc(NOD::DiscBase& disc, bool force,
                                 FProgress progress)=0;

    /* Basic path check (game directory matching) */
    virtual bool checkPathPrefix(const HECL::ProjectPath& path)=0;

    /* Pre-cook handlers */
    virtual bool validateYAMLDNAType(FILE* fp) const=0;

    /* Cook handlers */
    using BlendStream = HECL::BlenderConnection::DataStream;
    using Mesh = BlendStream::Mesh;

    virtual void cookMesh(const HECL::ProjectPath& out, const HECL::ProjectPath& in, BlendStream& ds, bool fast, FCookProgress progress) const=0;
    virtual void cookActor(const HECL::ProjectPath& out, const HECL::ProjectPath& in, BlendStream& ds, bool fast, FCookProgress progress) const=0;
    virtual void cookArea(const HECL::ProjectPath& out, const HECL::ProjectPath& in, BlendStream& ds, bool fast, FCookProgress progress) const=0;
    virtual void cookYAML(const HECL::ProjectPath& out, const HECL::ProjectPath& in, FILE* fin, FCookProgress progress) const=0;

    const HECL::ProjectPath& getMasterShaderPath() const {return m_masterShader;}

    SpecBase(HECL::Database::Project& project)
    : m_project(project),
      m_masterShader(project.getProjectWorkingPath(), ".hecl/RetroMasterShader.blend") {}
protected:
    HECL::Database::Project& m_project;
    HECL::ProjectPath m_masterShader;
private:
    std::unique_ptr<NOD::DiscBase> m_disc;
    bool m_isWii;
    bool m_standalone;
};

}

#endif // SPECBASE_HPP

