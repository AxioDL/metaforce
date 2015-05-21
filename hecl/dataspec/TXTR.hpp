#ifndef TXTR_HPP
#define TXTR_HPP

#include "HECLDatabase.hpp"

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
