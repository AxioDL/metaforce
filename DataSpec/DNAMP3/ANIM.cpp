#include "ANIM.hpp"
#include <float.h>
#include <math.h>

namespace DataSpec
{
namespace DNAMP3
{

using ANIMOutStream = HECL::BlenderConnection::PyOutStream::ANIMOutStream;

void ANIM::IANIM::sendANIMToBlender(HECL::BlenderConnection::PyOutStream& os, const CINF& cinf, bool additive) const
{
    os.format("act.hecl_fps = round(%f)\n"
              "act.hecl_additive = %s\n",
              1.0f / mainInterval, additive ? "True" : "False");

    auto kit = chanKeys.begin() + 1;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        const std::string* bName = cinf.getBoneNameFromId(bone.first);
        if (!bName)
        {
            if (std::get<0>(bone.second))
                ++kit;
            if (std::get<1>(bone.second))
                ++kit;
            if (std::get<2>(bone.second))
                ++kit;
            continue;
        }

        os.format("bone_string = '%s'\n", bName->c_str());
        os <<     "action_group = act.groups.new(bone_string)\n"
                  "\n";

        if (std::get<0>(bone.second))
            os << "rotCurves = []\n"
                  "rotCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].rotation_quaternion', index=0, action_group=bone_string))\n"
                  "rotCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].rotation_quaternion', index=1, action_group=bone_string))\n"
                  "rotCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].rotation_quaternion', index=2, action_group=bone_string))\n"
                  "rotCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].rotation_quaternion', index=3, action_group=bone_string))\n"
                  "\n";

        if (std::get<1>(bone.second))
        {
            if (!additive)
                os << "bone_trans_head = (0.0,0.0,0.0)\n"
                      "if arm_obj.data.bones[bone_string].parent is not None:\n"
                      "    bone_trans_head = Vector(arm_obj.data.bones[bone_string].head_local) - Vector(arm_obj.data.bones[bone_string].parent.head_local)\n";
            else
                os << "bone_trans_head = (0.0,0.0,0.0)\n";
            os << "transCurves = []\n"
                  "transCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].location', index=0, action_group=bone_string))\n"
                  "transCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].location', index=1, action_group=bone_string))\n"
                  "transCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].location', index=2, action_group=bone_string))\n"
                  "\n";
        }

        if (std::get<2>(bone.second))
            os << "scaleCurves = []\n"
                  "scaleCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].scale', index=0, action_group=bone_string))\n"
                  "scaleCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].scale', index=1, action_group=bone_string))\n"
                  "scaleCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].scale', index=2, action_group=bone_string))\n"
                  "\n";

        os << "crv = act.fcurves.new('pose.bones[\"'+bone_string+'\"].rotation_mode', action_group=bone_string)\n"
              "crv.keyframe_points.add()\n"
              "crv.keyframe_points[-1].co = (0, 0)\n"
              "crv.keyframe_points[-1].interpolation = 'LINEAR'\n"
              "\n";

        ANIMOutStream ao = os.beginANIMCurve();
        if (std::get<0>(bone.second))
        {
            const std::vector<DNAANIM::Value>& rotKeys = *kit++;
            for (int c=0 ; c<4 ; ++c)
            {
                auto frameit = frames.begin();
                ao.changeCurve(ANIMOutStream::CurveType::Rotate, c, rotKeys.size());
                for (const DNAANIM::Value& val : rotKeys)
                    ao.write(*frameit++, val.v4.vec[c]);
            }
        }

        if (std::get<1>(bone.second))
        {
            const std::vector<DNAANIM::Value>& transKeys = *kit++;
            for (int c=0 ; c<3 ; ++c)
            {
                auto frameit = frames.begin();
                ao.changeCurve(ANIMOutStream::CurveType::Translate, c, transKeys.size());
                for (const DNAANIM::Value& val : transKeys)
                    ao.write(*frameit++, val.v3.vec[c]);
            }
        }

        if (std::get<2>(bone.second))
        {
            const std::vector<DNAANIM::Value>& scaleKeys = *kit++;
            for (int c=0 ; c<3 ; ++c)
            {
                auto frameit = frames.begin();
                ao.changeCurve(ANIMOutStream::CurveType::Scale, c, scaleKeys.size());
                for (const DNAANIM::Value& val : scaleKeys)
                    ao.write(*frameit++, val.v3.vec[c]);
            }
        }
    }
}

void ANIM::ANIM0::read(Athena::io::IStreamReader& reader)
{
    Header head;
    head.read(reader);
    mainInterval = head.interval;

    frames.clear();
    frames.reserve(head.keyCount);
    for (size_t k=0 ; k<head.keyCount ; ++k)
        frames.push_back(k);

    std::map<atUint8, atUint32> boneMap;
    for (size_t b=0 ; b<head.boneSlotCount ; ++b)
    {
        atUint8 idx = reader.readUByte();
        if (idx == 0xff)
            continue;
        boneMap[idx] = b;
    }

    atUint32 boneCount = reader.readUint32Big();
    bones.clear();
    bones.reserve(boneCount);
    for (size_t b=0 ; b<boneCount ; ++b)
    {
        bones.emplace_back(boneMap[b], std::make_tuple(false, false, false));
        atUint8 idx = reader.readUByte();
        if (idx != 0xff)
            std::get<0>(bones.back().second) = true;
    }

    boneCount = reader.readUint32Big();
    for (size_t b=0 ; b<boneCount ; ++b)
    {
        atUint8 idx = reader.readUByte();
        if (idx != 0xff)
            std::get<1>(bones[b].second) = true;
    }

    boneCount = reader.readUint32Big();
    for (size_t b=0 ; b<boneCount ; ++b)
    {
        atUint8 idx = reader.readUByte();
        if (idx != 0xff)
            std::get<2>(bones[b].second) = true;
    }

    channels.clear();
    chanKeys.clear();
    channels.emplace_back();
    channels.back().type = DNAANIM::Channel::Type::KfHead;
    chanKeys.emplace_back();
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::Rotation;
            chanKeys.emplace_back();
        }
        if (std::get<1>(bone.second))
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::Translation;
            chanKeys.emplace_back();
        }
        if (std::get<2>(bone.second))
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::Scale;
            chanKeys.emplace_back();
        }
    }

    reader.readUint32Big();
    auto kit = chanKeys.begin() + 1;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
            ++kit;
        if (std::get<1>(bone.second))
            ++kit;
        if (std::get<2>(bone.second))
        {
            std::vector<DNAANIM::Value>& keys = *kit++;
            for (size_t k=0 ; k<head.keyCount ; ++k)
                keys.emplace_back(reader.readVec3fBig());
        }
    }

    reader.readUint32Big();
    kit = chanKeys.begin() + 1;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
        {
            std::vector<DNAANIM::Value>& keys = *kit++;
            for (size_t k=0 ; k<head.keyCount ; ++k)
                keys.emplace_back(reader.readVec4fBig());
        }
        if (std::get<1>(bone.second))
            ++kit;
        if (std::get<2>(bone.second))
            ++kit;
    }

    reader.readUint32Big();
    kit = chanKeys.begin() + 1;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
            ++kit;
        if (std::get<1>(bone.second))
        {
            std::vector<DNAANIM::Value>& keys = *kit++;
            for (size_t k=0 ; k<head.keyCount ; ++k)
                keys.emplace_back(reader.readVec3fBig());
        }
        if (std::get<2>(bone.second))
            ++kit;
    }
}

void ANIM::ANIM0::write(Athena::io::IStreamWriter& writer) const
{
    Header head;
    head.unk0 = 0;
    head.unk1 = 0;
    head.unk2 = 0;
    head.keyCount = frames.size();
    head.duration = head.keyCount * mainInterval;
    head.interval = mainInterval;

    atUint32 maxId = 0;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
        maxId = std::max(maxId, bone.first);
    head.boneSlotCount = maxId + 1;
    head.write(writer);

    for (size_t s=0 ; s<head.boneSlotCount ; ++s)
    {
        size_t boneIdx = 0;
        bool found = false;
        for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
        {
            if (s == bone.first)
            {
                writer.writeUByte(boneIdx);
                found = true;
                break;
            }
            ++boneIdx;
        }
        if (!found)
            writer.writeUByte(0xff);
    }

    writer.writeUint32Big(bones.size());
    size_t boneIdx = 0;
    size_t rotKeyCount = 0;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
        {
            writer.writeUByte(boneIdx);
            ++rotKeyCount;
        }
        else
            writer.writeUByte(0xff);
        ++boneIdx;
    }

    writer.writeUint32Big(bones.size());
    boneIdx = 0;
    size_t transKeyCount = 0;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<1>(bone.second))
        {
            writer.writeUByte(boneIdx);
            ++transKeyCount;
        }
        else
            writer.writeUByte(0xff);
        ++boneIdx;
    }

    writer.writeUint32Big(bones.size());
    boneIdx = 0;
    size_t scaleKeyCount = 0;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<2>(bone.second))
        {
            writer.writeUByte(boneIdx);
            ++scaleKeyCount;
        }
        else
            writer.writeUByte(0xff);
        ++boneIdx;
    }

    writer.writeUint32Big(scaleKeyCount * head.keyCount);
    auto cit = chanKeys.begin();
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
            ++cit;
        if (std::get<1>(bone.second))
            ++cit;
        if (std::get<2>(bone.second))
        {
            const std::vector<DNAANIM::Value>& keys = *cit++;
            auto kit = keys.begin();
            for (size_t k=0 ; k<head.keyCount ; ++k)
                writer.writeVec3fBig((*kit++).v3);
        }
    }

    writer.writeUint32Big(rotKeyCount * head.keyCount);
    cit = chanKeys.begin();
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
        {
            const std::vector<DNAANIM::Value>& keys = *cit++;
            auto kit = keys.begin();
            for (size_t k=0 ; k<head.keyCount ; ++k)
                writer.writeVec4fBig((*kit++).v4);
        }
        if (std::get<1>(bone.second))
            ++cit;
        if (std::get<2>(bone.second))
            ++cit;
    }

    writer.writeUint32Big(transKeyCount * head.keyCount);
    cit = chanKeys.begin();
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
            ++cit;
        if (std::get<1>(bone.second))
        {
            const std::vector<DNAANIM::Value>& keys = *cit++;
            auto kit = keys.begin();
            for (size_t k=0 ; k<head.keyCount ; ++k)
                writer.writeVec3fBig((*kit++).v3);
        }
        if (std::get<2>(bone.second))
            ++cit;
    }
}

size_t ANIM::ANIM0::binarySize(size_t __isz) const
{
    Header head;

    atUint32 maxId = 0;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
        maxId = std::max(maxId, bone.first);

    __isz = head.binarySize(__isz);
    __isz += maxId + 1;
    __isz += bones.size() * 3 + 12;

    __isz += 12;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
            __isz += head.keyCount * 16;
        if (std::get<1>(bone.second))
            __isz += head.keyCount * 12;
        if (std::get<2>(bone.second))
            __isz += head.keyCount * 12;
    }

    return __isz;
}

static float ComputeFrames(const std::vector<float>& keyTimes, std::vector<atUint32>& framesOut)
{
    if (keyTimes.size() <= 1)
        return 0.0;

    float mainInterval = FLT_MAX;
    float lastTime = keyTimes[0];
    for (auto it=keyTimes.begin() + 1 ; it != keyTimes.end() ; ++it)
    {
        float diff = *it - lastTime;
        if (diff < mainInterval)
            mainInterval = diff;
        lastTime = *it;
    }

    float fps = round(1.0 / mainInterval);
    if (fps < 15.0)
        fps = 15.0;
    mainInterval = 1.0 / fps;

    framesOut.clear();
    framesOut.reserve(keyTimes.size());
    atUint32 frameAccum = 0;
    for (float time : keyTimes)
    {
        while (frameAccum * mainInterval < time)
            ++frameAccum;
        framesOut.push_back(frameAccum);
    }

    return mainInterval;
}

void ANIM::ANIM1::read(Athena::io::IStreamReader& reader)
{
    Header head;
    head.read(reader);

    std::vector<float> keyTimes;
    keyTimes.reserve(head.keyCount);
    for (size_t k=0 ; k<head.keyCount ; ++k)
        keyTimes.push_back(reader.readFloatBig());
    mainInterval = ComputeFrames(keyTimes, frames);

    atUint8 boneFlagCount = reader.readUByte();
    bones.clear();
    bones.reserve(boneFlagCount);
    atUint32 boneChannelCount = 0;
    for (atUint8 f=0 ; f<boneFlagCount ; ++f)
    {
        atUint8 flag = reader.readUByte();
        bones.emplace_back(f, std::make_tuple(flag & 0x1, flag & 0x2, flag & 0x4));
        if (flag & 0x1)
            ++boneChannelCount;
        if (flag & 0x2)
            ++boneChannelCount;
        if (flag & 0x4)
            ++boneChannelCount;
    }

    std::vector<atInt16> initBlock;
    initBlock.reserve(head.initBlockSize/2);
    for (size_t i=0 ; i<head.initBlockSize/2 ; ++i)
        initBlock.push_back(reader.readInt16Big());

    atUint32 rawChannelCount = reader.readUint32Big();
    atUint32 scratchSize1 = reader.readUint32Big();
    atUint32 scratchSize2 = reader.readUint32Big();

    std::vector<atUint8> chanBitCounts;
    chanBitCounts.reserve(rawChannelCount);
    for (size_t c=0 ; c<rawChannelCount ; ++c)
        chanBitCounts.push_back(reader.readUByte());

    channels.clear();
    channels.reserve(boneChannelCount + 1);
    channels.emplace_back();
    channels.back().type = DNAANIM::Channel::Type::KfHead;
    auto initsIt = initBlock.begin();
    auto bitsIt = chanBitCounts.begin();
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::RotationMP3;
            chan.i[0] = *initsIt++;
            chan.q[0] = *bitsIt++;
            chan.i[1] = *initsIt++;
            chan.q[1] = *bitsIt++;
            chan.i[2] = *initsIt++;
            chan.q[2] = *bitsIt++;
            chan.i[3] = *initsIt++;
            chan.q[3] = *bitsIt++;
        }

        if (std::get<1>(bone.second))
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::Translation;
            chan.i[0] = *initsIt++;
            chan.q[0] = *bitsIt++;
            chan.i[1] = *initsIt++;
            chan.q[1] = *bitsIt++;
            chan.i[2] = *initsIt++;
            chan.q[2] = *bitsIt++;
        }

        if (std::get<2>(bone.second))
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::Scale;
            chan.i[0] = *initsIt++;
            chan.q[0] = *bitsIt++;
            chan.i[1] = *initsIt++;
            chan.q[1] = *bitsIt++;
            chan.i[2] = *initsIt++;
            chan.q[2] = *bitsIt++;
        }
    }

    size_t bsSize = DNAANIM::ComputeBitstreamSize(head.keyCount-1, channels);
    std::unique_ptr<atUint8[]> bsData = reader.readUBytes(bsSize);
    DNAANIM::BitstreamReader bsReader;
    chanKeys = bsReader.read(bsData.get(), head.keyCount-1, channels, 32767, head.translationMult);
}

void ANIM::ANIM1::write(Athena::io::IStreamWriter& writer) const
{
}

size_t ANIM::ANIM1::binarySize(size_t __isz) const
{
    return __isz;
}

}
}
