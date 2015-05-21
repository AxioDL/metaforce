#ifndef CMEMORYDATABASE_HPP
#define CMEMORYDATABASE_HPP

#include "HECLDatabase.hpp"
#include "CSQLite.hpp"

namespace HECLDatabase
{

class CMemoryDatabase final : public IDatabase
{
    CSQLite m_sql;
    Access m_access;
public:
    CMemoryDatabase(Access access)
    : m_sql(":memory:", (m_access == A_READONLY) ? true : false), m_access(access)
    {

    }

    ~CMemoryDatabase()
    {

    }

    Type getType() const
    {
        return T_MEMORY;
    }

    Access getAccess() const
    {
        return m_access;
    }

    const IDataObject* lookupObject(size_t id) const
    {
    }

    const IDataObject* lookupObject(const std::string& name) const
    {
    }

    const IDataObject* addDataBlob(const std::string& name, const void* data, size_t length)
    {
    }

    const IDataObject* addDataBlob(const void* data, size_t length)
    {
    }

    bool writeDatabase(IDatabase::Type type, const std::string& path) const
    {
    }

};

}

#endif // CMEMORYDATABASE_HPP
