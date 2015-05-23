#ifndef HECLDATABASE_HPP
#define HECLDATABASE_HPP

#include <iterator>
#include <string>
#include <functional>
#include <vector>
#include <stdexcept>
#include <stdint.h>

#include "HECL.hpp"

namespace HECLDatabase
{

class IDatabase;
class IProject;

/**
 * @brief Generic Database Object Class
 *
 * This abstract base-class is a typeless object node for entities in an
 * underlying database.
 */
class IDataObject
{
public:
    /**
     * @brief Data-key of object
     * @return Primary key
     */
    virtual int64_t id() const=0;

    /**
     * @brief FourCC type of object
     * @return FourCC type
     */
    virtual const HECL::FourCC& type() const=0;

    /**
     * @brief Data-hash of object
     * @return Object hash truncated to system's size-type
     */
    virtual const HECL::ObjectHash& hash() const=0;

    /**
     * @brief Original path of object
     * @return Name
     */
    virtual const std::string& path() const=0;

    /**
     * @brief Retrieve the database this object is stored within
     * @return database object
     */
    virtual IDatabase* parentDatabase() const=0;
};

/**
 * @brief An iterable collection of objects tracked within the database
 */
class IDataDependencyGroup
{
public:
    /**
     * @brief Count of objects in the group
     * @return object count
     */
    virtual size_t length() const=0;

    /**
     * @brief Alias of length()
     * @return object count
     */
    inline size_t size() const {return length();}

    /**
     * @brief Retrieve object at specified internal index within the group
     * @param idx internal index of object to fetch (range [0,length()-1])
     * @return object or nullptr
     */
    virtual const IDataObject* at(size_t idx) const=0;
    inline const IDataObject* operator[](size_t idx) {return at(idx);}

    virtual std::vector<IDataObject*>::const_iterator begin() const=0;
    virtual std::vector<IDataObject*>::const_iterator end() const=0;
};

/**
 * @brief Root database interface
 */
class IDatabase
{
public:
    virtual ~IDatabase() {}

    /**
     * @brief Database backend type
     */
    enum Type
    {
        T_UNKNOWN,
        T_MEMORY, /**< In-memory database; ideal for gathering small groups of frequently-accessed objects */
        T_LOOSE, /**< Loose database; ideal for read/write database construction or platforms with good filesystems */
        T_PACKED /**< Packed database; ideal for read-only archived data */
    };
    virtual Type getType() const=0;

    /**
     * @brief Database access type
     */
    enum Access
    {
        A_INVALID,
        A_READONLY, /**< Read-only access; packed databases always use this mode */
        A_READWRITE /**< Read/write access; used for building fresh databases */
    };
    virtual Access getAccess() const=0;

    /**
     * @brief Lookup object by database primary-key
     * @param id Primary-key of object
     * @return Data object
     */
    virtual const IDataObject* lookupObject(size_t id) const=0;

    /**
     * @brief Lookup object by name
     * @param name Name of object
     * @return Data object
     */
    virtual const IDataObject* lookupObject(const std::string& name) const=0;

    /**
     * @brief Write a full copy of the database to another type/path
     * @param type Type of new database
     * @param path Target path of new database
     * @return True on success
     */
    virtual bool writeDatabase(IDatabase::Type type, const std::string& path) const=0;

};

/**
 * @brief Creates a new (empty) database
 * @param type Type of new database
 * @param access Requested level of access
 * @return New database object
 *
 * Generally, the preferred method for working with HECL databases is via the
 * IProject interface. NewProject() will automatically construct the necessary
 * internal database objects.
 */
IDatabase* NewDatabase(IDatabase::Type type, IDatabase::Access access, const std::string& path);


/**
 * @brief Base object to subclass for integrating with key project operations
 *
 * All project objects are provided with IDataObject pointers to their database
 * entries. Subclasses register themselves with a type registry so instances
 * are automatically constructed when performing operations like cooking and packaging.
 *
 * DO NOT CONSTRUCT THIS OR SUBCLASSES DIRECTLY!!
 */
class CProjectObject
{
protected:
    friend class CProject;

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

    typedef std::function<void(const void* data, size_t len)> TDataAppender;

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
    virtual bool _cookObject(TDataAppender dataAppender,
                             DataEndianness endianness, DataPlatform platform)
    {(void)dataAppender;(void)endianness;(void)platform;return true;}

    typedef std::function<void(CProjectObject*)> TDepAdder;

    /**
     * @brief Optional private method implemented by CProjectObject subclasses to resolve dependencies
     * @param depAdder subclass calls this function zero or more times to register each dependency
     *
     * This method is called during IProject::packagePath().
     * Dependencies registered via this method will eventually have this method called on themselves
     * as well. This is a non-recursive operation, no need for subclasses to implement recursion-control.
     */
    virtual void _gatherDeps(TDepAdder depAdder)
    {(void)depAdder;}

protected:
    std::string m_path;
    IDataObject* m_mainObj;
    IDataObject* m_cookedObj;
public:
    static bool ClaimPath(const std::string&, const std::string&) {return false;}
    virtual ~CProjectObject();
    struct ConstructionInfo
    {
        IDataObject* mainObj;
        IDataObject* cookedObj;
        const std::string& path;
    };
    CProjectObject(const ConstructionInfo& info)
    : m_path(info.path), m_mainObj(info.mainObj), m_cookedObj(info.cookedObj) {}
};


/**
 * @brief Main project interface
 *
 * Projects are intermediate working directories used for staging
 * resources in their ideal editor-formats. This interface exposes all
 * primary operations to perform on a given project.
 */
class IProject
{
public:
    virtual ~IProject() {}

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
     * @brief Access internal database interface for working files
     * @return main working database object
     *
     * It's generally recommended for HECL frontends to avoid modifying
     * databases via this returned object.
     */
    virtual IDatabase* mainDatabase() const=0;

    /**
     * @brief Access internal database interface for cooked objects
     * @return main cooked database object
     *
     * It's generally recommended for HECL frontends to avoid modifying
     * databases via this returned object.
     */
    virtual IDatabase* cookedDatabase() const=0;

    /**
     * @brief Register an optional callback to report log-messages using
     * @param logger logger-callback
     *
     * If this method is never called, all project operations will run silently.
     */
    virtual void registerLogger(HECL::TLogger logger)=0;

    /**
     * @brief Get the path of the project's root-directory
     * @param absolute return as absolute-path
     * @return project root path
     *
     * Self explanatory
     */
    virtual std::string getProjectRootPath(bool absolute=false) const=0;

    /**
     * @brief Determine if an arbitrary absolute or relative path lies within project
     * @param subpath directory or file to validate
     * @return true if valid
     */
    virtual bool validateSubPath(const std::string& subpath) const=0;

    /**
     * @brief Add a given file or file-pattern to the database
     * @param path file or pattern within project
     * @return true on success
     *
     * This method blocks while object hashing takes place
     */
    virtual bool addPath(const std::string& path)=0;

    /**
     * @brief Remove a given file or file-pattern from the database
     * @param path file or pattern within project
     * @param recursive traverse into matched subdirectories
     * @return true on success
     *
     * This method will not delete actual working files from the project
     * directory. It will delete associated cooked objects though.
     */
    virtual bool removePath(const std::string& path, bool recursive=false)=0;

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
    virtual bool addGroup(const std::string& path)=0;

    /**
     * @brief Unregister a working sub-directory as a dependency group
     * @param path directory to unregister as Dependency Group
     * @return true on success
     */
    virtual bool removeGroup(const std::string& path)=0;

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
    virtual bool cookPath(const std::string& path,
                          std::function<void(std::string&, Cost, unsigned)> feedbackCb,
                          bool recursive=false)=0;

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
    virtual void interruptCook()=0;

    /**
     * @brief Delete cooked objects for directory
     * @param path directory of intermediates to clean
     * @param recursive traverse subdirectories to clean as well
     * @return true on success
     *
     * Developers understand how useful 'clean' is. While ideally not required,
     * it's useful for verifying that a rebuild from ground-up is doable.
     */
    virtual bool cleanPath(const std::string& path, bool recursive=false)=0;

    /**
     * @brief Package cooked objects for directory
     * @param path directory of intermediates to package
     * @param recursive traverse subdirectories to package as well
     * @return true on success
     *
     * Once all dependent resources are cooked, this method archives specified
     * intermediates into a packed database file located alongside the specified
     * directory. This is a similar process to 'linking' in software development.
     *
     * Part of this process involves calling CProjectObject::_gatherDeps() to calculate
     * object dependencies. This makes package-assembly simple, as dependencies will
     * automatically be added as needed. The frontend needn't be concerned about
     * gathering leaf-objects buried in corners of the working directory.
     */
    virtual bool packagePath(const std::string& path, bool recursive=false)=0;

};

/**
 * @brief Creates a new (empty) project using specified root directory
 * @param rootPath Path to project root-directory (may be relative)
 * @return New project object
 *
 * This is the preferred way to open an existing or create a new HECL project.
 * All necessary database index files and object directories will be established
 * within the specified directory path.
 */
IProject* NewProject(const std::string& rootPath);


/**
 * @brief Base object to subclass for integrating with key runtime operations
 *
 * All runtime objects are provided with IDataObject pointers to their database
 * entries. Subclasses register themselves with a type registry so instances
 * are automatically constructed when performing operations like runtime-integration.
 *
 * DO NOT CONSTRUCT THIS OR SUBCLASSES DIRECTLY!!
 */
class CRuntimeObject
{
    unsigned m_refCount = 0;
    bool m_loaded = false;
protected:

    /**
     * @brief Optional subclass method called on background thread or in response to interrupt when data is ready
     * @param data fully-loaded data buffer
     * @param len length of buffer
     * @return true when data is successfully integrated into the runtime
     */
    virtual bool _objectFinishedLoading(const void* data, size_t len)
    {(void)data;(void)len;return true;}

    /**
     * @brief Optional subclass method called in response to reference-count dropping to 0
     */
    virtual void _objectWillUnload() {}

protected:
    IDataObject* m_obj;
public:
    struct ConstructionInfo
    {
        IDataObject* obj;
    };
    CRuntimeObject(const ConstructionInfo& info)
    : m_obj(info.obj) {}

    /**
     * @brief Determine if object is fully loaded and constructed
     * @return true if so
     */
    inline bool isLoaded() const {return m_loaded;}

    /**
     * @brief Increment object's reference count
     * @return true if already loaded (ready to use) false if load is staged (poll with isLoaded())
     *
     * CRuntimeObject instances initially have an internal reference-count of 0.
     * By calling this method from 0, an asynchronous load operation takes place.
     * Synchronous loads are discouraged by HECL in order to avoid stalling game
     * systems. Please poll with isLoaded() to keep things running smoothly!
     */
    bool incRef();

    /**
     * @brief Decrement object's reference count
     *
     * If the internal reference-count reaches 0, the object's unload procedure takes place
     */
    void decRef();

};

/**
 * @brief Runtime data-management interface
 *
 * Interface for controlling runtime data-operations like object lookup
 * and burst load-transactions. The runtime's implementation automatically
 * constructs CRuntimeObject instances as needed.
 */
class IRuntime
{
public:
    virtual ~IRuntime() {}

    /**
     * @brief Lookup singular object by database ID
     * @param id database ID of object
     * @return runtime object
     */
    virtual CRuntimeObject* lookupObjectById(size_t id);

    /**
     * @brief Iterable group view providing a load interface for Dependency Groups
     *
     * HECL uses a background thread or other asynchronous loading mechanism to
     * efficiently load and construct IRuntimeObject instances.
     *
     * The iterator interface may be used immediately to access contained objects.
     */
    class IStagedGroup
    {
    public:
        /**
         * @brief Poll to see if transaction complete
         * @return true if complete
         */
        virtual bool isDone() const=0;

        virtual std::vector<CRuntimeObject*>::iterator begin() const=0;
        virtual std::vector<CRuntimeObject*>::iterator end() const=0;
    };

    /**
     * @brief Begin asynchronously loading a dependency group by id
     * @param groupId the id of the dependency group within the database
     * @return Staged group interface scheduled to load ASAP
     */
    virtual IStagedGroup* loadDependencyGroup(int64_t groupId);

    /**
     * @brief Unload a previously-loaded dependency group or cancel a load in-progress
     * @param group Staged Group obtained via loadDependencyGroup()
     */
    virtual void unloadDependencyGroup(IStagedGroup* group);
};

/**
 * @brief Statically-constructed structure registering a FourCC with project
 * and runtime factories. This is used for constructing key operational subclasses
 * for cooking/packaging during development and runtime-integrating during gameplay.
 */
struct RegistryEntry
{
    typedef std::function<bool(const std::string& path)> TPathClaimer;
    typedef std::function<CProjectObject*(const CProjectObject::ConstructionInfo&)> TProjectFactory;
    typedef std::function<CRuntimeObject*(const CRuntimeObject::ConstructionInfo&)> TRuntimeFactory;
    const HECL::FourCC& fcc;
#ifndef HECL_STRIP_PROJECT
    TPathClaimer pathClaimer;
    TProjectFactory projectFactory;
#endif
#ifndef HECL_STRIP_RUNTIME
    TRuntimeFactory runtimeFactory;
#endif
};

static RegistryEntry::TPathClaimer NULL_PATH_CLAIMER =
    [](const std::string&) -> bool {return false;};
static RegistryEntry::TProjectFactory NULL_PROJECT_FACTORY =
    [](const HECLDatabase::CProjectObject::ConstructionInfo&)
    -> HECLDatabase::CProjectObject* {return nullptr;};
static RegistryEntry::TRuntimeFactory NULL_RUNTIME_FACTORY =
    [](const HECLDatabase::CRuntimeObject::ConstructionInfo&)
    -> HECLDatabase::CRuntimeObject* {return nullptr;};

#if !defined(HECL_STRIP_PROJECT) && !defined(HECL_STRIP_RUNTIME)

#define REGISTRY_ENTRY(fourcc, projectClass, runtimeClass) {fourcc, \
[](const std::string& path) -> bool {return projectClass::ClaimPath(path);}, \
[](const HECLDatabase::CProjectObject::ConstructionInfo& info) -> \
    HECLDatabase::CProjectObject* {return new projectClass(info);}, \
[](const HECLDatabase::CRuntimeObject::ConstructionInfo& info) -> \
    HECLDatabase::CRuntimeObject* {return new runtimeClass(info);}}

#define REGISTRY_SENTINEL() \
    {HECL::FourCC(), NULL_PATH_CLAIMER, \
    NULL_PROJECT_FACTORY, NULL_RUNTIME_FACTORY}

#elif !defined(HECL_STRIP_PROJECT)

#define REGISTRY_ENTRY(fourcc, projectClass, runtimeClass) {fourcc, \
[](const std::string& path) -> bool {return projectClass::ClaimPath(path);}, \
[](const HECLDatabase::CProjectObject::ConstructionInfo& info) -> \
    HECLDatabase::CProjectObject* {return new projectClass(info);}}

#define REGISTRY_SENTINEL() {HECL::FourCC(), NULL_PATH_CLAIMER, NULL_PROJECT_FACTORY}

#elif !defined(HECL_STRIP_RUNTIME)

#define REGISTRY_ENTRY(fourcc, projectClass, runtimeClass) {fourcc, \
[](const HECLDatabase::CRuntimeObject::ConstructionInfo& info) -> \
    HECLDatabase::CRuntimeObject* {return new runtimeClass(info);}}

#define REGISTRY_SENTINEL() {HECL::FourCC(), NULL_RUNTIME_FACTORY}

#endif

/**
 * @brief Statically-constructed table of registered types
 *
 * Table is defined in dataspec/dataspec.cpp of HECL's codebase.
 * Developers are encouraged to modify/extend the default data model as
 * required by their project.
 *
 * The REGISTRY_ENTRY macro is a helper for defining entries.
 * The REGISTRY_SENTINEL must be inserted at the end of the table.
 */
extern const RegistryEntry DATASPEC_TYPE_REGISTRY[];

}

#endif // HECLDATABASE_HPP
