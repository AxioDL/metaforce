#ifndef DUMB_HPP
#define DUMB_HPP

#include "HECLRuntime.hpp"

class CDUMBProject : public HECLDatabase::ProjectObjectBase
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

};

class CDUMBRuntime : public HECLRuntime::RuntimeObjectBase
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

#endif // DUMB_HPP
