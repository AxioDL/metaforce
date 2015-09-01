#include "ANIM.hpp"

namespace Retro
{
namespace DNAMP2
{

using ANIMOutStream = HECL::BlenderConnection::PyOutStream::ANIMOutStream;

void ANIM::IANIM::sendANIMToBlender(HECL::BlenderConnection::PyOutStream& os, const CINF& cinf) const
{
    os.format("act.hecl_fps = round(%f)\n", (1.0f / mainInterval));

    auto kit = chanKeys.begin();
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        const std::string* bName = cinf.getBoneNameFromId(bone.first);
        if (!bName)
            continue;
        
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
            os << "bone_trans_head = (0.0,0.0,0.0)\n"
                  "if arm_obj.data.bones[bone_string].parent is not None:\n"
                  "    bone_trans_head = Vector(arm_obj.data.bones[bone_string].head_local) - Vector(arm_obj.data.bones[bone_string].parent.head_local)\n"
                  "transCurves = []\n"
                  "transCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].location', index=0, action_group=bone_string))\n"
                  "transCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].location', index=1, action_group=bone_string))\n"
                  "transCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].location', index=2, action_group=bone_string))\n"
                  "\n";

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
                ao.changeCurve(ANIMOutStream::CurveRotate, c, rotKeys.size());
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
                ao.changeCurve(ANIMOutStream::CurveTranslate, c, transKeys.size());
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
                ao.changeCurve(ANIMOutStream::CurveScale, c, scaleKeys.size());
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
            std::get<1>(bones.back().second) = true;
    }

    boneCount = reader.readUint32Big();
    for (size_t b=0 ; b<boneCount ; ++b)
    {
        atUint8 idx = reader.readUByte();
        if (idx != 0xff)
            std::get<2>(bones.back().second) = true;
    }

    channels.clear();
    chanKeys.clear();
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        if (std::get<0>(bone.second))
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::ROTATION;
            chanKeys.emplace_back();
        }
        if (std::get<1>(bone.second))
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::TRANSLATION;
            chanKeys.emplace_back();
        }
        if (std::get<2>(bone.second))
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::SCALE;
            chanKeys.emplace_back();
        }
    }

    reader.readUint32Big();
    auto kit = chanKeys.begin();
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
    kit = chanKeys.begin();
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
    kit = chanKeys.begin();
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
        maxId = MAX(maxId, bone.first);
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

void ANIM::ANIM2::read(Athena::io::IStreamReader& reader)
{
    Header head;
    head.read(reader);
    mainInterval = head.interval;

    WordBitmap keyBmp;
    keyBmp.read(reader, head.keyBitmapBitCount);
    frames.clear();
    atUint32 frameAccum = 0;
    for (bool bit : keyBmp)
    {
        if (bit)
            frames.push_back(frameAccum);
        ++frameAccum;
    }
    reader.seek(4);

    bones.clear();
    bones.reserve(head.boneChannelCount);
    channels.clear();
    channels.reserve(head.boneChannelCount);
    size_t keyframeCount = 0;
    for (size_t b=0 ; b<head.boneChannelCount ; ++b)
    {
        ChannelDesc desc;
        desc.read(reader);
        bones.emplace_back(desc.id, std::make_tuple(desc.keyCount1 != 0, desc.keyCount2 != 0, desc.keyCount3 != 0));

        if (desc.keyCount1)
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::ROTATION;
            chan.i[0] = desc.initRX;
            chan.q[0] = desc.qRX;
            chan.i[1] = desc.initRY;
            chan.q[1] = desc.qRY;
            chan.i[2] = desc.initRZ;
            chan.q[2] = desc.qRZ;
        }
        keyframeCount = MAX(keyframeCount, desc.keyCount1);

        if (desc.keyCount2)
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::TRANSLATION;
            chan.i[0] = desc.initTX;
            chan.q[0] = desc.qTX;
            chan.i[1] = desc.initTY;
            chan.q[1] = desc.qTY;
            chan.i[2] = desc.initTZ;
            chan.q[2] = desc.qTZ;
        }
        keyframeCount = MAX(keyframeCount, desc.keyCount2);

        if (desc.keyCount3)
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::SCALE;
            chan.i[0] = desc.initSX;
            chan.q[0] = desc.qSX;
            chan.i[1] = desc.initSY;
            chan.q[1] = desc.qSY;
            chan.i[2] = desc.initSZ;
            chan.q[2] = desc.qSZ;
        }
        keyframeCount = MAX(keyframeCount, desc.keyCount3);
    }

    size_t bsSize = DNAANIM::ComputeBitstreamSize(keyframeCount, channels);
    std::unique_ptr<atUint8[]> bsData = reader.readUBytes(bsSize);
    DNAANIM::BitstreamReader bsReader;
    chanKeys = bsReader.read(bsData.get(), keyframeCount, channels, head.rotDiv, head.translationMult);
}

void ANIM::ANIM2::write(Athena::io::IStreamWriter& writer) const
{
    Header head;
    head.unk1 = 1;
    head.unk2 = 1;
    head.interval = mainInterval;
    head.unk3 = 0;
    head.unk4 = 0;
    head.unk5 = 0;
    head.unk6 = 1;

    WordBitmap keyBmp;
    size_t frameCount = 0;
    for (atUint32 frame : frames)
    {
        while (keyBmp.getBit(frame))
            ++frame;
        keyBmp.setBit(frame);
        frameCount = frame + 1;
    }
    head.keyBitmapBitCount = frameCount;
    head.duration = frameCount * mainInterval;
    head.boneChannelCount = bones.size();

    size_t keyframeCount = frames.size();
    std::vector<DNAANIM::Channel> qChannels = channels;
    DNAANIM::BitstreamWriter bsWriter;
    size_t bsSize;
    std::unique_ptr<atUint8[]> bsData = bsWriter.write(chanKeys, keyframeCount, qChannels,
                                                       head.rotDiv, head.translationMult, bsSize);

    /* TODO: Figure out proper scratch size computation */
    head.scratchSize = keyframeCount * channels.size() * 16;

    head.write(writer);
    keyBmp.write(writer);
    writer.writeUint32Big(head.boneChannelCount);
    auto cit = qChannels.begin();
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        ChannelDesc desc;
        if (std::get<0>(bone.second))
        {
            DNAANIM::Channel& chan = *cit++;
            desc.keyCount1 = keyframeCount;
            desc.initRX = chan.i[0];
            desc.qRX = chan.q[0];
            desc.initRY = chan.i[1];
            desc.qRY = chan.q[1];
            desc.initRZ = chan.i[2];
            desc.qRZ = chan.q[2];
        }
        if (std::get<1>(bone.second))
        {
            DNAANIM::Channel& chan = *cit++;
            desc.keyCount2 = keyframeCount;
            desc.initTX = chan.i[0];
            desc.qTX = chan.q[0];
            desc.initTY = chan.i[1];
            desc.qTY = chan.q[1];
            desc.initTZ = chan.i[2];
            desc.qTZ = chan.q[2];
        }
        if (std::get<2>(bone.second))
        {
            DNAANIM::Channel& chan = *cit++;
            desc.keyCount3 = keyframeCount;
            desc.initSX = chan.i[0];
            desc.qSX = chan.q[0];
            desc.initSY = chan.i[1];
            desc.qSY = chan.q[1];
            desc.initSZ = chan.i[2];
            desc.qSZ = chan.q[2];
        }
    }

    writer.writeUBytes(bsData.get(), bsSize);
}

}
}
