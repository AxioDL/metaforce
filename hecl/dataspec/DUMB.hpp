#ifndef DUMB_HPP
#define DUMB_HPP

#include "HECLDatabase.hpp"

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

class CDUMBRuntime : public HECLDatabase::RuntimeObjectBase
{
    using HECLDatabase::RuntimeObjectBase::RuntimeObjectBase;

    bool _objectFinishedLoading(const void* data, size_t len)
    {
        return true;
    }

    void _objectWillUnload()
    {

    }

};

#endif // DUMB_HPP
