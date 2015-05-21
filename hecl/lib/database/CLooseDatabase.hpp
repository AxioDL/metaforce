#ifndef CLOOSEDATABASE_HPP
#define CLOOSEDATABASE_HPP

#include <stdio.h>
#include <blowfish/blowfish.h>
#include <zlib/zlib.h>

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
    : m_sql(path.c_str(), (access == A_READONLY) ? true : false),
      m_access(access)
    {

    }

    ~CLooseDatabase()
    {

    }

    Type getType() const
    {
        return T_LOOSE;
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
        /* Hash data */
        HECL::ObjectHash hash(data, length);

        /* Compress data into file */
        FILE* fp = fopen("", "wb");

        m_sql.insertObject(name, "DUMB", hash, length, length);
    }

    const IDataObject* addDataBlob(const void* data, size_t length)
    {
        return addDataBlob(std::string(), data, length);
    }

    bool writeDatabase(IDatabase::Type type, const std::string& path) const
    {
        if (type == T_PACKED)
        {
            size_t bufSz;
            void* buf = m_sql.fillDBBuffer(bufSz);
            FILE* fp = fopen(path.c_str(), "wb");
            fwrite(buf, 1, bufSz, fp);
            return true;
        }
        return false;
    }

};

}

#endif // CLOOSEDATABASE_HPP
