#ifndef CPACKEDDATABASE_HPP
#define CPACKEDDATABASE_HPP

#include "HECLDatabase.hpp"
#include "CSQLite.hpp"

namespace HECLDatabase
{

class CPackedDatabase final : public IDatabase
{
    CSQLite m_sql;
public:
    CPackedDatabase(const std::string& path)
    : m_sql(path.c_str(), true)
    {

    }

    ~CPackedDatabase()
    {

    }

    Type getType() const
    {
        return T_PACKED;
    }

    Access getAccess() const
    {
        return A_READONLY;
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

#endif // CPACKEDDATABASE_HPP
