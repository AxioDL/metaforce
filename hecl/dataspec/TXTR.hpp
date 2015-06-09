#ifndef TXTR_HPP
#define TXTR_HPP

#include "HECLDatabase.hpp"
#include "HECLRuntime.hpp"
#include "helpers.hpp"

class CTXTRProject : public HECLDatabase::ProjectObjectBase
{
    using HECLDatabase::ProjectObjectBase::ProjectObjectBase;

    bool _cookObject(FDataAppender dataAppender,
                     DataEndianness endianness, DataPlatform platform)
    {
        return true;
    }

    void _gatherDeps(FDepAdder depAdder)
    {

    }

public:
    static bool ClaimPath(const std::string& path, const std::string&)
    {
        if (!HECLHelpers::IsRegularFile(path))
            return false;
        if (!HECLHelpers::ContainsMagic(path, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8))
            return false;
        return true;
    }
};

class CTXTRRuntime : public HECLRuntime::RuntimeObjectBase
{
    using HECLRuntime::RuntimeObjectBase::RuntimeObjectBase;

    bool _objectFinishedLoading(const void* data, size_t len)
    {
        return true;
    }

    void _objectWillUnload()
    {

    }
};

#endif // TXTR_HPP
