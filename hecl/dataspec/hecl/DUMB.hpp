#ifndef DUMB_HPP
#define DUMB_HPP

#include "HECLDatabase.hpp"
#include "HECLRuntime.hpp"

class CDUMBProject : public HECL::Database::ObjectBase
{
    using HECL::Database::ObjectBase::ObjectBase;

    bool _cookObject(FDataAppender dataAppender,
                     DataEndianness endianness, DataPlatform platform)
    {
        return true;
    }

    void _gatherDeps(FDepAdder depAdder)
    {

    }

};

class CDUMBRuntime : public HECL::Runtime::ObjectBase
{
    using HECL::Runtime::ObjectBase::ObjectBase;

    bool _objectFinishedLoading(const void* data, size_t len)
    {
        return true;
    }

    void _objectWillUnload()
    {

    }

};

#endif // DUMB_HPP
