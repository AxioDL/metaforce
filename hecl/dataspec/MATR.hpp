#ifndef MATR_HPP
#define MATR_HPP

#include "HECLDatabase.hpp"
#include "HECLRuntime.hpp"

class CMATRProject : public HECLDatabase::ProjectObjectBase
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

class CMATRRuntime : public HECLRuntime::RuntimeObjectBase
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

#endif // MATR_HPP
