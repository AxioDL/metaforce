#ifndef HECLDATABASE_HPP
#define HECLDATABASE_HPP

#include <iterator>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <atomic>
#include <stdexcept>
#include <stdint.h>

#include "HECL.hpp"

namespace HECLDatabase
{

/**
 * @brief Base object to subclass for integrating with key project operations
 *
 * All project objects are provided with IDataObject pointers to their database
 * entries. Subclasses register themselves with a type registry so instances
 * are automatically constructed when performing operations like cooking and packaging.
 *
 * DO NOT CONSTRUCT THIS OR SUBCLASSES DIRECTLY!!
 */
class ProjectObjectBase
{
    friend class CProject;
    std::string m_path;
protected:

    /**
     * @brief Byte-order of target system
     */
    enum DataEndianness
    {
        DE_NONE,
        DE_BIG, /**< Big-endian (PowerPC) */
        DE_LITTLE /**< Little-endian (Intel) */
    };

    /**
     * @brief Data-formats of target system
     */
    enum DataPlatform
    {
        DP_NONE,
        DP_GENERIC, /**< Scanline textures and 3-way shader bundle (GLSL, HLSL, SPIR-V) */
        DP_REVOLUTION, /**< Tiled textures and GX register buffers */
        DP_CAFE /**< Swizzled textures and R700 shader objects */
    };

    typedef std::function<void(const void* data, size_t len)> FDataAppender;

    /**
     * @brief Optional private method implemented by CProjectObject subclasses to cook objects
     * @param dataAppender subclass calls this function zero or more times to provide cooked-data linearly
     * @param endianness byte-order to target
     * @param platform data-formats to target
     * @return true if cook succeeded
     *
     * This method is called during IProject::cookPath().
     * Part of the cooking process may include embedding database-refs to dependencies.
     * This method should store the 64-bit value provided by IDataObject::id() when doing this.
     */
    virtual bool _cookObject(FDataAppender dataAppender,
                             DataEndianness endianness, DataPlatform platform)
    {(void)dataAppender;(void)endianness;(void)platform;return true;}

    typedef std::function<void(ProjectObjectBase*)> FDepAdder;

    /**
     * @brief Optional private method implemented by CProjectObject subclasses to resolve dependencies
     * @param depAdder subclass calls this function zero or more times to register each dependency
     *
     * This method is called during IProject::packagePath().
     * Dependencies registered via this method will eventually have this method called on themselves
     * as well. This is a non-recursive operation, no need for subclasses to implement recursion-control.
     */
    virtual void _gatherDeps(FDepAdder depAdder)
    {(void)depAdder;}

public:
    ProjectObjectBase(const std::string& path)
    : m_path(path) {}

    inline const std::string& getPath() const {return m_path;}

    /**
     * @brief Overridable function to verify data at provided path
     * @return true if ProjectObject subclass handles data at provided path/subpath
     */
    static bool ClaimPath(const std::string& /*path*/, const std::string& /*subpath*/) {return false;}

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
    HECL::SystemString m_rootPath;
public:
    Project(const std::string& rootPath);

    class ConfigFile
    {
        const Project& m_project;
        const HECL::SystemString& m_name;
        HECL::SystemString m_filepath;
    public:
        ConfigFile(const Project& project, const HECL::SystemString& name);
        std::vector<std::string> readLines();
        void addLine(const std::string& line);
        void removeLine(const std::string& refLine);
        bool checkForLine(const std::string& refLine);
    };

    /**
     * @brief Internal packagePath() exception
     *
     * Due to the recursive nature of packagePath(), there are potential
     * pitfalls like infinite-recursion. HECL throws this whenever there
     * are uncooked dependencies or if the maximum dependency-recursion
     * limit is exceeded.
     */
    class PackageException : public std::runtime_error {};

    /**
     * @brief A rough description of how 'expensive' a given cook operation is
     *
     * This is used to provide pretty colors during the cook operation
     */
    enum Cost
    {
        C_NONE,
        C_LIGHT,
        C_MEDIUM,
        C_HEAVY
    };

    /**
     * @brief Register an optional callback to report log-messages using
     * @param logger logger-callback
     *
     * If this method is never called, all project operations will run silently.
     */
    virtual void registerLogger(HECL::TLogger logger);

    /**
     * @brief Get the path of the project's root-directory
     * @param absolute return as absolute-path
     * @return project root path
     *
     * Self explanatory
     */
    virtual const HECL::ProjectRootPath& getProjectRootPath(bool absolute=false) const;

    /**
     * @brief Add given file(s) to the database
     * @param path file or pattern within project
     * @return true on success
     *
     * This method blocks while object hashing takes place
     */
    virtual bool addPaths(const std::vector<HECL::ProjectPath>& paths);

    /**
     * @brief Remove a given file or file-pattern from the database
     * @param paths file(s) or pattern(s) within project
     * @param recursive traverse into matched subdirectories
     * @return true on success
     *
     * This method will not delete actual working files from the project
     * directory. It will delete associated cooked objects though.
     */
    virtual bool removePaths(const std::vector<HECL::ProjectPath>& paths, bool recursive=false);

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
    virtual bool addGroup(const HECL::ProjectPath& path);

    /**
     * @brief Unregister a working sub-directory as a dependency group
     * @param path directory to unregister as Dependency Group
     * @return true on success
     */
    virtual bool removeGroup(const HECL::ProjectPath& path);

    /**
     * @brief Return map populated with platforms targetable by this project interface
     * @return Platform map with name-string keys and enable-status values
     */
    virtual const std::map<const std::string, const bool>& listPlatforms();

    /**
     * @brief Enable persistent user preference for particular platform string(s)
     * @param platforms String(s) representing unique platform(s) from listPlatforms
     * @return true on success
     */
    virtual bool enablePlatforms(const std::vector<std::string>& platforms);

    /**
     * @brief Disable persistent user preference for particular platform string(s)
     * @param platform String(s) representing unique platform(s) from listPlatforms
     * @return true on success
     */
    virtual bool disablePlatforms(const std::vector<std::string>& platforms);

    /**
     * @brief Begin cook process for specified directory
     * @param path directory of intermediates to cook
     * @param feedbackCb a callback to run reporting cook-progress
     * @param recursive traverse subdirectories to cook as well
     * @return true on success
     *
     * Object cooking is generally an expensive process for large projects.
     * This method blocks execution during the procedure, with periodic
     * feedback delivered via feedbackCb.
     */
    virtual bool cookPath(const HECL::SystemString& path,
                          std::function<void(std::string&, Cost, unsigned)> feedbackCb,
                          bool recursive=false);

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
    virtual void interruptCook();

    /**
     * @brief Delete cooked objects for directory
     * @param path directory of intermediates to clean
     * @param recursive traverse subdirectories to clean as well
     * @return true on success
     *
     * Developers understand how useful 'clean' is. While ideally not required,
     * it's useful for verifying that a rebuild from ground-up is doable.
     */
    virtual bool cleanPath(const HECL::SystemString& path, bool recursive=false);

    /**
     * @brief Nodegraph class for gathering dependency-resolved objects for packaging
     */
    class PackageDepsgraph
    {
    public:
        struct Node
        {
            enum
            {
                NODE_DATA,
                NODE_GROUP
            } type;
            std::string path;
            ProjectObjectBase* projectObj;
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
     * @brief Constructs a full depsgraph of the project-subpath provided
     * @param path Subpath of project to root depsgraph at
     * @return Populated depsgraph ready to traverse
     */
    virtual PackageDepsgraph buildPackageDepsgraph(const HECL::ProjectPath& path);

};


/**
 * @brief Subclassed by dataspec entries to manage per-game aspects of the data pipeline
 *
 * The DataSpec class manages interfaces for unpackaging, cooking, and packaging
 * of data for interacting with a specific system/game-engine.
 */
class IDataSpec
{
public:

    virtual Project::PackageDepsgraph packageData();
};

}

#endif // HECLDATABASE_HPP
