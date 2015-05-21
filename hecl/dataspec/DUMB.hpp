#ifndef DUMB_HPP
#define DUMB_HPP

#include "HECLDatabase.hpp"

class CDUMBProject : public HECLDatabase::CProjectObject
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
    CDUMBProject(const ConstructionInfo& info)
    : CProjectObject(info)
    {
    }

    ~CDUMBProject()
    {
    }
};

class CDUMBRuntime : public HECLDatabase::CRuntimeObject
{

    bool _objectFinishedLoading(const void* data, size_t len)
    {
        return true;
    }

    void _objectWillUnload()
    {

    }

public:
    CDUMBRuntime(const ConstructionInfo& info)
    : CRuntimeObject(info)
    {
    }
    ~CDUMBRuntime()
    {
    }
};

#endif // DUMB_HPP
