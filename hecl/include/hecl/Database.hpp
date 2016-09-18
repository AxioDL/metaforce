#ifndef HECLDATABASE_HPP
#define HECLDATABASE_HPP

#include <iterator>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <atomic>
#include <fstream>
#include <stdint.h>
#include <assert.h>

#include <athena/IStreamReader.hpp>
#include "logvisor/logvisor.hpp"

#include "hecl.hpp"

namespace hecl
{
class BlenderToken;

namespace Database
{
class Project;

extern logvisor::Module LogModule;

typedef std::function<void(const hecl::SystemChar*, const hecl::SystemChar*, int, float)> FProgress;

/**
 * @brief Nodegraph class for gathering dependency-resolved objects for packaging
 */
class PackageDepsgraph
{
public:
    struct Node
    {
        enum class Type
        {
            Data,
            Group
        } type;
        ProjectPath path;
        ProjectPath cookedPath;
        class ObjectBase* projectObj;
        Node* sub;
        Node* next;
    };
private:
    friend class Project;
    std::vector<Node> m_nodes;
public:
    const Node* getRootNode() const {return &m_nodes[0];}
};

/**
 * @brief Subclassed by dataspec entries to manage per-game aspects of the data pipeline
 *
 * The DataSpec class manages interfaces for unpackaging, cooking, and packaging
 * of data for interacting with a specific system/game-engine.
 */
class IDataSpec
{
    const DataSpecEntry* m_specEntry;
public:
    IDataSpec(const DataSpecEntry* specEntry) : m_specEntry(specEntry) {}
    virtual ~IDataSpec() {}
    using FProgress = hecl::Database::FProgress;
    using FCookProgress = std::function<void(const SystemChar*)>;

    /**
     * @brief Extract Pass Info
     *
     * An extract pass iterates through a source package or image and
     * reverses the cooking process by emitting editable resources
     */
    struct ExtractPassInfo
    {
        SystemString srcpath;
        std::vector<SystemString> extractArgs;
        bool force;
    };

    /**
     * @brief Extract Report Representation
     *
     * Constructed by canExtract() to advise the user of the content about
     * to be extracted
     */
    struct ExtractReport
    {
        SystemString name;
        SystemString desc;
        std::vector<ExtractReport> childOpts;
    };

    virtual void setThreadProject() {}

    virtual bool canExtract(const ExtractPassInfo& info, std::vector<ExtractReport>& reps)
    {(void)info;(void)reps;LogModule.report(logvisor::Error, "not implemented");return false;}
    virtual void doExtract(const ExtractPassInfo& info, FProgress progress)
    {(void)info;(void)progress;}

    virtual bool canCook(const ProjectPath& path, BlenderToken& btok)
    {(void)path;LogModule.report(logvisor::Error, "not implemented");return false;}
    virtual const DataSpecEntry* overrideDataSpec(const ProjectPath& path,
                                                  const Database::DataSpecEntry* oldEntry,
                                                  BlenderToken& btok)
    {(void)path;return oldEntry;}
    virtual void doCook(const ProjectPath& path, const ProjectPath& cookedPath,
                        bool fast, BlenderToken& btok, FCookProgress progress)
    {(void)path;(void)cookedPath;(void)fast;(void)progress;}

    /**
     * @brief Package Pass Info
     *
     * A package pass performs last-minute queries of source resources,
     * gathers dependencies and packages cooked data together in the
     * most efficient form for the dataspec
     */
    struct PackagePassInfo
    {
        const PackageDepsgraph& depsgraph;
        ProjectPath subpath;
        ProjectPath outpath;
    };
    virtual bool canPackage(const PackagePassInfo& info,
                            SystemString& reasonNo)
    {(void)info;reasonNo=_S("not implemented");return false;}
    virtual void gatherDependencies(const PackagePassInfo& info,
                                    std::unordered_set<ProjectPath>& implicitsOut)
    {(void)info;(void)implicitsOut;}
    virtual void doPackage(const PackagePassInfo& info)
    {(void)info;}

    const DataSpecEntry* getDataSpecEntry() const {return m_specEntry;}
};

/**
 * @brief Pre-emptive indication of what the constructed DataSpec is used for
 */
enum class DataSpecTool
{
    Extract,
    Cook,
    Package
};

extern std::vector<const struct DataSpecEntry*> DATA_SPEC_REGISTRY;

/**
 * @brief IDataSpec registry entry
 *
 * Auto-registers with data spec registry
 */
struct DataSpecEntry
{
    const SystemChar* m_name;
    const SystemChar* m_desc;
    std::function<IDataSpec*(Project&, DataSpecTool)> m_factory;

    DataSpecEntry(const SystemChar* name, const SystemChar* desc,
                  std::function<IDataSpec*(Project& project, DataSpecTool)>&& factory)
    : m_name(name), m_desc(desc), m_factory(std::move(factory)) {}
};

/**
 * @brief Base object to subclass for integrating with key project operations
 *
 * All project objects are provided with IDataObject pointers to their database
 * entries. Subclasses register themselves with a type registry so instances
 * are automatically constructed when performing operations like cooking and packaging.
 *
 * DO NOT CONSTRUCT THIS OR SUBCLASSES DIRECTLY!!
 */
class ObjectBase
{
    friend class Project;
    SystemString m_path;
protected:

    /**
     * @brief Byte-order of target system
     */
    enum class DataEndianness
    {
        None,
        Big, /**< Big-endian (PowerPC) */
        Little /**< Little-endian (Intel) */
    };

    /**
     * @brief Data-formats of target system
     */
    enum class DataPlatform
    {
        None,
        Generic, /**< Scanline textures and 3-way shader bundle (GLSL, HLSL, SPIR-V) */
        Revolution, /**< Tiled textures and GX register buffers */
        Cafe /**< Swizzled textures and R700 shader objects */
    };

    typedef std::function<void(const void* data, size_t len)> FDataAppender;

    /**
     * @brief Optional private method implemented by subclasses to cook objects
     * @param dataAppender subclass calls this function zero or more times to provide cooked-data linearly
     * @param endianness byte-order to target
     * @param platform data-formats to target
     * @return true if cook succeeded
     *
     * This method is called during IProject::cookPath().
     * Part of the cooking process may include embedding database-refs to dependencies.
     * This method should store the 64-bit value provided by IDataObject::id() when doing this.
     */
    virtual bool cookObject(FDataAppender dataAppender,
                            DataEndianness endianness, DataPlatform platform)
    {(void)dataAppender;(void)endianness;(void)platform;return true;}

    typedef std::function<void(ObjectBase*)> FDepAdder;

    /**
     * @brief Optional private method implemented by CProjectObject subclasses to resolve dependencies
     * @param depAdder subclass calls this function zero or more times to register each dependency
     *
     * This method is called during IProject::packagePath().
     * Dependencies registered via this method will eventually have this method called on themselves
     * as well. This is a non-recursive operation, no need for subclasses to implement recursion-control.
     */
    virtual void gatherDeps(FDepAdder depAdder)
    {(void)depAdder;}

    /**
     * @brief Get a packagable FourCC representation of the object's type
     * @return FourCC of the type
     */
    virtual FourCC getType() const
    {return FourCC("NULL");}

public:
    ObjectBase(const SystemString& path)
    : m_path(path) {}

    const SystemString& getPath() const {return m_path;}

};


/**
 * @brief Main project interface
 *
 * Projects are intermediate working directories used for staging
 * resources in their ideal editor-formats. This interface exposes all
 * primary operations to perform on a given project.
 */
class Project
{
public:
    struct ProjectDataSpec
    {
        const DataSpecEntry& spec;
        ProjectPath cookedPath;
        bool active;
    };
private:
    ProjectRootPath m_rootPath;
    ProjectPath m_workRoot;
    ProjectPath m_dotPath;
    ProjectPath m_cookedRoot;
    std::vector<ProjectDataSpec> m_compiledSpecs;
    bool m_valid = false;
public:
    Project(const hecl::ProjectRootPath& rootPath);
    operator bool() const {return m_valid;}

    /**
     * @brief Configuration file handle
     *
     * Holds a path to a line-delimited textual configuration file;
     * opening a locked handle for read/write transactions
     */
    class ConfigFile
    {
        SystemString m_filepath;
        std::vector<std::string> m_lines;
        FILE* m_lockedFile = NULL;
    public:
        ConfigFile(const Project& project, const SystemString& name,
                   const SystemString& subdir=_S("/.hecl/"));
        std::vector<std::string>& lockAndRead();
        void addLine(const std::string& line);
        void removeLine(const std::string& refLine);
        bool checkForLine(const std::string& refLine);
        void unlockAndDiscard();
        bool unlockAndCommit();
    };
    ConfigFile m_specs;
    ConfigFile m_paths;
    ConfigFile m_groups;

    /**
     * @brief A rough description of how 'expensive' a given cook operation is
     *
     * This is used to provide pretty colors during the cook operation
     */
    enum class Cost
    {
        None,
        Light,
        Medium,
        Heavy
    };

    /**
     * @brief Get the path of the project's root-directory
     * @return project root path
     *
     * Self explanatory
     */
    const ProjectRootPath& getProjectRootPath() const {return m_rootPath;}

    /**
     * @brief Get the path of project's working directory
     * @return project working path
     */
    const ProjectPath& getProjectWorkingPath() const {return m_workRoot;}

    /**
     * @brief Get the path of project's cooked directory for a specific DataSpec
     * @param DataSpec to retrieve path for
     * @return project cooked path
     *
     * The cooked path matches the directory layout of the working directory
     */
    const ProjectPath& getProjectCookedPath(const DataSpecEntry& spec) const;

    /**
     * @brief Add given file(s) to the database
     * @param path file or pattern within project
     * @return true on success
     *
     * This method blocks while object hashing takes place
     */
    bool addPaths(const std::vector<ProjectPath>& paths);

    /**
     * @brief Remove a given file or file-pattern from the database
     * @param paths file(s) or pattern(s) within project
     * @param recursive traverse into matched subdirectories
     * @return true on success
     *
     * This method will not delete actual working files from the project
     * directory. It will delete associated cooked objects though.
     */
    bool removePaths(const std::vector<ProjectPath>& paths, bool recursive=false);

    /**
     * @brief Register a working sub-directory as a Dependency Group
     * @param path directory to register as Dependency Group
     * @return true on success
     *
     * Dependency Groups are used at runtime to stage burst load-transactions.
     * They may only be added to directories and will automatically claim
     * subdirectories as well.
     *
     * Cooked objects in dependency groups will be packaged contiguously
     * and automatically duplicated if shared with other dependency groups.
     * This contiguous storage makes for optimal loading from slow block-devices
     * like optical drives.
     */
    bool addGroup(const ProjectPath& path);

    /**
     * @brief Unregister a working sub-directory as a dependency group
     * @param path directory to unregister as Dependency Group
     * @return true on success
     */
    bool removeGroup(const ProjectPath& path);

    /**
     * @brief Re-reads the data store holding user's spec preferences
     *
     * Call periodically in a long-term use of the hecl::Database::Project class.
     * Install filesystem event-hooks if possible.
     */
    void rescanDataSpecs();

    /**
     * @brief Return map populated with dataspecs targetable by this project interface
     * @return Platform map with name-string keys and enable-status values
     */
    const std::vector<ProjectDataSpec>& getDataSpecs() const {return m_compiledSpecs;}

    /**
     * @brief Enable persistent user preference for particular spec string(s)
     * @param specs String(s) representing unique spec(s) from listDataSpecs
     * @return true on success
     */
    bool enableDataSpecs(const std::vector<SystemString>& specs);

    /**
     * @brief Disable persistent user preference for particular spec string(s)
     * @param specs String(s) representing unique spec(s) from listDataSpecs
     * @return true on success
     */
    bool disableDataSpecs(const std::vector<SystemString>& specs);

    /**
     * @brief Begin cook process for specified directory
     * @param path directory of intermediates to cook
     * @param feedbackCb a callback to run reporting cook-progress
     * @param recursive traverse subdirectories to cook as well
     * @param fast enables faster (draft) extraction for supported data types
     * @return true on success
     *
     * Object cooking is generally an expensive process for large projects.
     * This method blocks execution during the procedure, with periodic
     * feedback delivered via feedbackCb.
     */
    bool cookPath(const ProjectPath& path, FProgress feedbackCb,
                  bool recursive=false, bool force=false, bool fast=false);

    /**
     * @brief Interrupts a cook in progress (call from SIGINT handler)
     *
     * Database corruption is bad! sqlite is pretty robust at avoiding data corruption,
     * but HECL spreads its data objects through the filesystem; this ensures that
     * those objects are cleanly finalized or discarded before stopping.
     *
     * Note that this method returns immediately; the resumed cookPath()
     * call will return as quickly as possible.
     */
    void interruptCook();

    /**
     * @brief Delete cooked objects for directory
     * @param path directory of intermediates to clean
     * @param recursive traverse subdirectories to clean as well
     * @return true on success
     *
     * Developers understand how useful 'clean' is. While ideally not required,
     * it's useful for verifying that a rebuild from ground-up is doable.
     */
    bool cleanPath(const ProjectPath& path, bool recursive=false);

    /**
     * @brief Constructs a full depsgraph of the project-subpath provided
     * @param path Subpath of project to root depsgraph at
     * @return Populated depsgraph ready to traverse
     */
    PackageDepsgraph buildPackageDepsgraph(const ProjectPath& path);

};

}
}

#endif // HECLDATABASE_HPP
