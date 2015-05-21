#ifndef HMDL_HPP
#define HMDL_HPP

#include "HECLDatabase.hpp"

class CHMDLProject : public HECLDatabase::CProjectObject
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
    CHMDLProject(const ConstructionInfo& info)
    : CProjectObject(info)
    {
    }

    ~CHMDLProject()
    {
    }
};

class CHMDLRuntime : public HECLDatabase::CRuntimeObject
{

    bool _objectFinishedLoading(const void* data, size_t len)
    {
        return true;
    }

    void _objectWillUnload()
    {

    }

public:
    CHMDLRuntime(const ConstructionInfo& info)
    : CRuntimeObject(info)
    {
    }
    ~CHMDLRuntime()
    {
    }
};

#endif // HMDL_HPP
