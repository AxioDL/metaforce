#ifndef __URDE_CINGAMETWEAKMANAGERBASE_HPP__
#define __URDE_CINGAMETWEAKMANAGERBASE_HPP__

#include <string>
#include <vector>

#include "RetroTypes.hpp"

namespace urde
{

class CTweakValue
{
public:
    struct Audio
    {
        Audio(float, float, float, const std::string&, u32);
        void None();
    };
    enum class EType
    {
    };


    EType x0_type;
    std::string x4_key;
    std::string x30_str;
public:
    CTweakValue()=default;
    //CTweakValue(CTextInputStream&);
    //void PutTo(CTextOutStream&);
    const std::string& GetName() const { return x4_key; }
    const std::string& GetValueAsString() const;
    void SetValueFromString(const std::string&);
    const Audio& GetAudio() const;
    EType GetType() const { return x0_type; }
};

class CInGameTweakManagerBase
{
protected:
    std::vector<CTweakValue> x0_values;
public:
    bool HasTweakValue(const std::string& name) const
    {
        for (const CTweakValue& val : x0_values)
            if (val.GetName() == name)
                return true;
        return false;
    }

    const CTweakValue* GetTweakValue(const std::string& name) const
    {
        for (const CTweakValue& val : x0_values)
            if (val.GetName() == name)
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
