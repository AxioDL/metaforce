#ifndef HECLDATABASE_CPP
#error This file must only be included from HECLDatabase.cpp
#endif

#include "HECLDatabase.hpp"
#include "CSQLite.hpp"

namespace HECLDatabase
{

class CLooseDatabase final : public IDatabase
{
    CSQLite m_sql;
    Access m_access;
public:
    CLooseDatabase(const std::string& path, Access access)
    : m_sql(path.c_str(), (m_access == READONLY) ? true : false), m_access(access)
    {

    }

    ~CLooseDatabase()
    {

    }

    Type getType() const
    {
        return LOOSE;
    }

    Access getAccess() const
    {
        return m_access;
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
