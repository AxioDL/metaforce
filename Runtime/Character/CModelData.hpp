#ifndef __URDE_CMODELDATA_HPP__
#define __URDE_CMODELDATA_HPP__

#include "zeus/CVector3f.hpp"

namespace urde
{

class CAnimRes
{
    zeus::CVector3f x8_scale;
public:
    CAnimRes();
};

class CModelData
{
public:
    CModelData(const CAnimRes& res);
};

}

#endif // __URDE_CMODELDATA_HPP__
