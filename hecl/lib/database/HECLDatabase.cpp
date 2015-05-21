

#include "HECLDatabase.hpp"

#include "CLooseDatabase.hpp"
#include "CPackedDatabase.hpp"
#include "CMemoryDatabase.hpp"

namespace HECLDatabase
{

IDatabase* NewDatabase(IDatabase::Type type, IDatabase::Access access, const std::string& path)
{
    switch (type)
    {
    case IDatabase::T_LOOSE:
        return new CLooseDatabase(path, access);
    case IDatabase::T_PACKED:
        return new CPackedDatabase(path);
    case IDatabase::T_MEMORY:
        return new CMemoryDatabase(access);
    case IDatabase::T_UNKNOWN:
        return nullptr;
    }
    return nullptr;
}

}
