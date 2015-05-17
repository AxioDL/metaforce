#ifndef HECLDATABASE_CPP
#error This file must only be included from HECLDatabase.cpp
#endif

#include "HECLDatabase.hpp"
#include "CSQLite.hpp"

namespace HECLDatabase
{

class CPackedDatabase final : public IDatabase
{
    CSQLite m_sql;
public:
    CPackedDatabase(const std::string& path)
    : m_sql(path.c_str(), (m_access == READONLY) ? true : false)
    {

    }

    ~CPackedDatabase()
    {

    }

    Type getType() const
    {
        return PACKED;
    }

    Access getAccess() const
    {
        return READONLY;
    }

    const IDataObject* lookupObject(std::size_t id) const
    {
    }

    const IDataObject* lookupObject(const std::string& name) const
    {
    }

    const IDataObject* addDataBlob(const std::string& name, const void* data, std::size_t length)
    {
    }

    const IDataObject* addDataBlob(const void* data, std::size_t length)
    {
    }

    bool writeDatabase(IDatabase::Type type, const std::string& path) const
    {
    }

};

}
