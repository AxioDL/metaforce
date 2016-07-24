#ifndef __URDE_CINGAMETWEAKMANAGERBASE_HPP__
#define __URDE_CINGAMETWEAKMANAGERBASE_HPP__

#include <string>
#include <vector>

namespace urde
{

struct CTweakValue
{
    enum class EType
    {
    } x0_type;
    std::string x4_key;
    std::string x30_str;
};

class CInGameTweakManagerBase
{
protected:
    std::vector<CTweakValue> x0_values;
public:
    bool HasTweakValue(const std::string& name) const
    {
        for (const CTweakValue& val : x0_values)
            if (val.x4_key == name)
                return true;
        return false;
    }

    const CTweakValue* GetTweakValue(const std::string& name) const
    {
        for (const CTweakValue& val : x0_values)
            if (val.x4_key == name)
                return &val;
        return nullptr;
    }

    bool ReadFromMemoryCard(const std::string& name)
    {
        return true;
    }
};

}

#endif // __URDE_CINGAMETWEAKMANAGERBASE_HPP__
