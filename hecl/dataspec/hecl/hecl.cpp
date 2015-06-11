#include "HECLDatabase.hpp"

namespace HECL
{
namespace Database
{

class HECLBaseDataSpec : public IDataSpec
{

};

class HECLLittleDataSpec : public HECLBaseDataSpec
{
};

class HECLBigDataSpec : public HECLBaseDataSpec
{
};

class HECLRevolutionDataSpec : public HECLBaseDataSpec
{
};

class HECLCafeDataSpec : public HECLBaseDataSpec
{
};

IDataSpec* NewDataSpec_little()
{
    return new HECLLittleDataSpec();
}

IDataSpec* NewDataSpec_big()
{
    return new HECLBigDataSpec();
}

IDataSpec* NewDataSpec_revolution()
{
    return new HECLRevolutionDataSpec();
}

IDataSpec* NewDataSpec_cafe()
{
    return new HECLCafeDataSpec();
}

}
}
