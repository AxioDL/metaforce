#include "HECLDatabase.hpp"

#define HECLDATABASE_CPP
#include "CLooseDatabase.hpp"
#include "CPackedDatabase.hpp"
#include "CMemoryDatabase.hpp"

namespace HECLDatabase
{

IDatabase* NewDatabase(IDatabase::Type type, IDatabase::Access access, const std::string& path)
{
    switch (type)
    {
    case IDatabase::LOOSE:
        return new CLooseDatabase(path, access);
    case IDatabase::PACKED:
        return new CPackedDatabase(path);
    case IDatabase::MEMORY:
        return new CMemoryDatabase(access);
    case IDatabase::UNKNOWN:
        return nullptr;
    }
    return nullptr;
}

}
