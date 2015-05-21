#ifndef STRG_HPP
#define STRG_HPP

#include "HECLDatabase.hpp"

class CSTRGProject : public HECLDatabase::CProjectObject
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
    CSTRGProject(const ConstructionInfo& info)
    : CProjectObject(info)
    {
    }

    ~CSTRGProject()
    {
    }
};

class CSTRGRuntime : public HECLDatabase::CRuntimeObject
{

    bool _objectFinishedLoading(const void* data, size_t len)
    {
        return true;
    }

    void _objectWillUnload()
    {

    }

public:
    CSTRGRuntime(const ConstructionInfo& info)
    : CRuntimeObject(info)
    {
    }
    ~CSTRGRuntime()
    {
    }
};

#endif // STRG_HPP
