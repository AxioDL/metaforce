#ifndef MATR_HPP
#define MATR_HPP

#include "HECLDatabase.hpp"

class CMATRProject : public HECLDatabase::CProjectObject
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
    CMATRProject(const ConstructionInfo& info)
    : CProjectObject(info)
    {
    }

    ~CMATRProject()
    {
    }
};

class CMATRRuntime : public HECLDatabase::CRuntimeObject
{

    bool _objectFinishedLoading(const void* data, size_t len)
    {
        return true;
    }

    void _objectWillUnload()
    {

    }

public:
    CMATRRuntime(const ConstructionInfo& info)
    : CRuntimeObject(info)
    {
    }
    ~CMATRRuntime()
    {
    }
};

#endif // MATR_HPP
