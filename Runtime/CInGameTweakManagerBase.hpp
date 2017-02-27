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
        float x0_fadeIn, x4_fadeOut, x8_volume;
        std::string xc_fileName;
        ResId x1c_res;
        Audio(float fadeIn, float fadeOut, float vol, const std::string& fileName, u32 handle)
        : x0_fadeIn(fadeIn), x4_fadeOut(fadeOut), x8_volume(vol), xc_fileName(fileName), x1c_res(handle) {}
        float GetFadeIn() const { return x0_fadeIn; }
        float GetFadeOut() const { return x4_fadeOut; }
        float GetVolume() const { return x8_volume; }
        const std::string& GetFileName() const { return xc_fileName; }
        ResId GetResId() const { return x1c_res; }
        static Audio None() { return Audio{0.f, 0.f, 0.f, "", 0}; }
    };
    enum class EType
    {
    };


    EType x0_type;
    std::string x4_key;
    std::string x14_str;
    Audio x24_audio;
    union
    {
        u32 x44_int;
        float x44_flt;
    };
public:
    CTweakValue()=default;
    //CTweakValue(CTextInputStream&);
    //void PutTo(CTextOutStream&);
    const std::string& GetName() const { return x4_key; }
    const std::string& GetValueAsString() const;
    void SetValueFromString(const std::string&);
    const Audio& GetAudio() const { return x24_audio; }
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
