#ifndef TXTR_HPP
#define TXTR_HPP

#include "HECLDatabase.hpp"
#include "helpers.hpp"

class CTXTRProject : public HECLDatabase::CProjectObject
{
    bool _cookObject(TDataAppender dataAppender,
                     DataEndianness endianness, DataPlatform platform)
    {
        return true;
    }

    void _gatherDeps(TDepAdder depAdder)
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

    CTXTRProject(const ConstructionInfo& info)
    : CProjectObject(info)
    {
    }

    ~CTXTRProject()
    {
    }
};

class CTXTRRuntime : public HECLDatabase::CRuntimeObject
{

    bool _objectFinishedLoading(const void* data, size_t len)
    {
        return true;
    }

    void _objectWillUnload()
    {

    }

public:
    CTXTRRuntime(const ConstructionInfo& info)
    : CRuntimeObject(info)
    {
    }
    ~CTXTRRuntime()
    {
    }
};

#endif // TXTR_HPP
