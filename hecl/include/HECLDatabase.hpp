#ifndef HECLDATABASE_HPP
#define HECLDATABASE_HPP

#include <iterator>
#include <string>

namespace HECLDatabase
{

class IDatabase;

/**
 * @brief The IDataObject class
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
    virtual std::size_t id() const=0;

    /**
     * @brief Textual name of object
     * @return Name
     */
    virtual const std::string& name() const=0;

    /**
     * @brief Data-hash of object
     * @return Object hash truncated to system's size-type
     */
    virtual std::size_t hash() const=0;

    /**
     * @brief Retrieve the database this object is stored within
     * @return database object
     */
    virtual IDatabase* parent() const=0;
};

/**
 * @brief An iterable collection of objects tracked within the database
 */
class IDataDependencyGroup : public IDataObject
{
public:
    /**
     * @brief Count of objects in the group
     * @return object count
     */
    virtual std::size_t length() const=0;

    /**
     * @brief Alias of length()
     * @return object count
     */
    inline std::size_t size() const {return length();}

    /**
     * @brief Retrieve object at specified internal index within the group
     * @param idx internal index of object to fetch (range [0,length()-1])
     * @return object or nullptr
     */
    virtual const IDataObject* at(std::size_t idx) const=0;
    inline const IDataObject* operator[](std::size_t idx) {return at(idx);}

    /**
     * @brief Simple IDataDependencyGroup iterator
     *
     * Use begin()/end() or C++11 ranged-for to use
     */
    class iterator : std::iterator<std::forward_iterator_tag, const IDataObject*>
    {
        friend class IDataDependencyGroup;
        const IDataDependencyGroup& m_grp;
        std::size_t m_idx = 0;
        inline iterator(const IDataDependencyGroup& grp) : m_grp(grp) {}
    public:
        inline bool operator!=(const iterator& other)
        {return &m_grp != &other.m_grp || m_idx != other.m_idx;}
        inline iterator& operator++() {++m_idx; return *this;}
    };
    inline iterator begin() const {return iterator(*this);}
    inline iterator end() const {auto it = iterator(*this); it.m_idx = length(); return it;}
};

/**
 * @brief A generic database object storing raw bytes
 */
class IDataBlob : public IDataObject
{
public:
    /**
     * @brief Accesses the length of the object (in bytes)
     * @return Data length
     */
    virtual std::size_t length() const=0;

    /**
     * @brief Alias for the length() function
     * @return Data length
     */
    inline std::size_t size() const {return length();}

    /**
     * @brief Accesses the object's data
     * @return Immutable pointer to object's data
     *
     * Note that the database may not have data loaded immediately loaded.
     * This function will perform a blocking-load of the data object.
     * Gather objects into an ILoadTransaction to pre-emptively load
     * collections of objects.
     */
    virtual const void* data() const=0;
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
        UNKNOWN,
        MEMORY, /**< In-memory database; ideal for gathering small groups of frequently-accessed objects */
        LOOSE, /**< Loose database; ideal for read/write database construction or platforms with good filesystems */
        PACKED /**< Packed database; ideal for read-only archived data */
    };
    virtual Type getType() const=0;

    /**
     * @brief Database access type
     */
    enum Access
    {
        INVALID,
        READONLY, /**< Read-only access; packed databases always use this mode */
        READWRITE /**< Read/write access; used for building fresh databases */
    };
    virtual Access getAccess() const=0;

    /**
     * @brief Lookup object by database primary-key
     * @param id Primary-key of object
     * @return Data object
     */
    virtual const IDataObject* lookupObject(std::size_t id) const=0;

    /**
     * @brief Lookup object by name
     * @param name Name of object
     * @return Data object
     */
    virtual const IDataObject* lookupObject(const std::string& name) const=0;

    /**
     * @brief Insert named data-blob object into a database with read/write access
     * @param name Name of object
     * @param data Pointer to object data (will be copied)
     * @param length Size of object data to copy
     * @return New data object
     */
    virtual const IDataObject* addDataBlob(const std::string& name, const void* data, std::size_t length)=0;

    /**
     * @brief Insert Data-blob object into a database with read/write access
     * @param data Pointer to object data (will be copied)
     * @param length size of object data to copy
     * @return New data object
     */
    virtual const IDataObject* addDataBlob(const void* data, std::size_t length)=0;

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
 */
IDatabase* NewDatabase(IDatabase::Type type, IDatabase::Access access, const std::string& path);

}

#endif // HECLDATABASE_HPP
