#ifndef HECLRUNTIME_HPP
#define HECLRUNTIME_HPP

#include <memory>
#include <vector>
#include <atomic>

#include "HECL.hpp"


namespace HECLRuntime
{

class RuntimeEntity
{
public:
    enum ERuntimeEntityType
    {
        ENTITY_NONE,
        ENTITY_OBJECT,
        ENTITY_GROUP
    };

private:
    ERuntimeEntityType m_type;
    const std::string& m_path;
    bool m_loaded = false;

    friend class RuntimeGroup;
    friend class RuntimeObjectBase;
    RuntimeEntity(ERuntimeEntityType type, const std::string& path)
    : m_type(type), m_path(path) {}

public:
    /**
     * @brief Get type of runtime object
     * @return Type enum
     */
    inline ERuntimeEntityType getType() const {return m_type;}

    /**
     * @brief Get database entity path
     * @return Path string
     */
    inline const std::string& getPath() const {return m_path;}

    /**
     * @brief Determine if object is fully loaded and constructed
     * @return true if so
     */
    inline bool isLoaded() const {return m_loaded;}
};

/**
 * @brief Interface representing a load-ordered group of runtime objects
 *
 * HLPK files perform all data retrieval using the notion of 'groups'
 * Groups are a collection of data objects that have been sequentially packed
 * in the package file and are constructed in the indexed order of the group.
 *
 * RuntimeGroup objects are internally created and weakly-referenced by CRuntime.
 * RuntimeObject objects are weakly-referenced by RuntimeGroup; they're strongly
 * referenced by application systems as long as they're needed.
 *
 * DO NOT CONSTRUCT THIS DIRECTLY!!
 */
class RuntimeGroup : public RuntimeEntity
{
public:
    typedef std::vector<std::weak_ptr<const class RuntimeObjectBase>> GroupObjectsVector;
private:
    friend class HECLRuntime;
    GroupObjectsVector m_objects;
    RuntimeGroup(const std::string& path)
    : RuntimeEntity(ENTITY_GROUP, path) {}
public:
    inline const GroupObjectsVector& getObjects() const {return m_objects;}
};

/**
 * @brief Base object to subclass for integrating with key runtime operations
 *
 * All runtime objects are provided with IDataObject pointers to their database
 * entries. Subclasses register themselves with a type registry so instances
 * are automatically constructed when performing operations like runtime-integration.
 *
 * DO NOT CONSTRUCT THIS OR SUBCLASSES DIRECTLY!!
 */
class RuntimeObjectBase : public RuntimeEntity
{
    std::shared_ptr<const RuntimeGroup> m_parent;
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

public:
    RuntimeObjectBase(const RuntimeGroup* group, const std::string& path)
    : RuntimeEntity(ENTITY_OBJECT, path), m_parent(group) {}

    /**
     * @brief Get parent group of object
     * @return Borrowed pointer of parent RuntimeGroup
     */
    inline const RuntimeGroup* getParentGroup() {return m_parent.get();}
};

/**
 * @brief HLPK Runtime data-management root
 *
 * Interface for controlling runtime data-operations like object lookup
 * and burst load-transactions using HLPK packages. The runtime's
 * implementation automatically constructs RuntimeObjectBase and
 * RuntimeGroup instances as needed.
 */
class HECLRuntime
{
public:
    /**
     * @brief Constructs the HECL runtime root
     * @param hlpkDirectory directory to search for .hlpk files
     */
    HECLRuntime(const HECL::SystemString& hlpkDirectory);
    ~HECLRuntime();

    /**
     * @brief Structure indicating the load status of an object group
     */
    struct SGroupLoadStatus
    {
        std::atomic_bool done;
        std::atomic_size_t completedObjects;
        std::atomic_size_t totalObjects;
    };

    /**
     * @brief Begin a synchronous group-load transaction
     * @param pathHash Hashed path string to perform lookup
     * @return Shared reference to the loading/loaded object
     *
     * This method blocks until the entire containing-group is loaded.
     * Paths to groups or individual objects are accepted.
     */
    std::shared_ptr<RuntimeEntity> loadSync(const HECL::Hash& pathHash);

    /**
     * @brief Begin an asynchronous group-load transaction
     * @param pathHash Hashed path string to perform lookup
     * @param statusOut Optional atomically-pollable structure updated with status fields
     * @return Shared reference to the loading/loaded object
     *
     * This method returns once all group entity stubs are constructed.
     * Paths to groups or individual objects are accepted.
     */
    std::shared_ptr<RuntimeEntity> loadAsync(const HECL::Hash& pathHash,
                                             SGroupLoadStatus* statusOut=NULL);

};

}

#endif // HECLRUNTIME_HPP
