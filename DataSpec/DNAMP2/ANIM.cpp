#include "ANIM.hpp"

namespace DataSpec
{
namespace DNAMP2
{

using ANIMOutStream = hecl::BlenderConnection::PyOutStream::ANIMOutStream;

void ANIM::IANIM::sendANIMToBlender(hecl::BlenderConnection::PyOutStream& os, const DNAANIM::RigInverter<CINF>& rig) const
{
    os.format("act.hecl_fps = round(%f)\n"
              "act.hecl_looping = %s\n",
              (1.0f / mainInterval), looping ? "True" : "False");

    auto kit = chanKeys.begin();

    std::vector<zeus::CQuaternion> fixedRotKeys;
    std::vector<zeus::CVector3f> fixedTransKeys;

    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        const std::string* bName = rig.getCINF().getBoneNameFromId(bone.first);
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
            os << "transCurves = []\n"
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

        ANIMOutStream ao = os.beginANIMCurve();
        if (std::get<0>(bone.second))
        {
            const std::vector<DNAANIM::Value>& rotKeys = *kit++;
            fixedRotKeys.clear();
            fixedRotKeys.resize(rotKeys.size());

            for (int c=0 ; c<4 ; ++c)
            {
                size_t idx = 0;
                for (const DNAANIM::Value& val : rotKeys)
                    fixedRotKeys[idx++][c] = val.v4.vec[c];
            }

            for (zeus::CQuaternion& rot : fixedRotKeys)
                rot = rig.invertRotation(bone.first, rot);

            for (int c=0 ; c<4 ; ++c)
            {
                auto frameit = frames.begin();
                ao.changeCurve(ANIMOutStream::CurveType::Rotate, c, rotKeys.size());
                for (const zeus::CQuaternion& val : fixedRotKeys)
                    ao.write(*frameit++, val[c]);
            }
        }

        if (std::get<1>(bone.second))
        {
            const std::vector<DNAANIM::Value>& transKeys = *kit++;
            fixedTransKeys.clear();
            fixedTransKeys.resize(transKeys.size());

            for (int c=0 ; c<3 ; ++c)
            {
                size_t idx = 0;
                for (const DNAANIM::Value& val : transKeys)
                    fixedTransKeys[idx++][c] = val.v3.vec[c];
            }

            for (zeus::CVector3f& t : fixedTransKeys)
                t = rig.invertPosition(bone.first, t, true);

            for (int c=0 ; c<3 ; ++c)
            {
                auto frameit = frames.begin();
                ao.changeCurve(ANIMOutStream::CurveType::Translate, c, fixedTransKeys.size());
                for (const zeus::CVector3f& val : fixedTransKeys)
                    ao.write(*frameit++, val[c]);
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

void ANIM::ANIM0::read(athena::io::IStreamReader& reader)
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

void ANIM::ANIM0::write(athena::io::IStreamWriter& writer) const
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

void ANIM::ANIM2::read(athena::io::IStreamReader& reader)
{
    Header head;
    head.read(reader);
    mainInterval = head.interval;
    looping = bool(head.looping);

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
    atUint16 keyframeCount = 0;
    for (size_t b=0 ; b<head.boneChannelCount ; ++b)
    {
        ChannelDesc desc;
        desc.read(reader);
        bones.emplace_back(desc.id, std::make_tuple(desc.keyCount1 != 0, desc.keyCount2 != 0, desc.keyCount3 != 0));

        if (desc.keyCount1)
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::Rotation;
            chan.i[0] = desc.initRX;
            chan.q[0] = desc.qRX;
            chan.i[1] = desc.initRY;
            chan.q[1] = desc.qRY;
            chan.i[2] = desc.initRZ;
            chan.q[2] = desc.qRZ;
        }
        keyframeCount = std::max(keyframeCount, desc.keyCount1);

        if (desc.keyCount2)
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::Translation;
            chan.i[0] = desc.initTX;
            chan.q[0] = desc.qTX;
            chan.i[1] = desc.initTY;
            chan.q[1] = desc.qTY;
            chan.i[2] = desc.initTZ;
            chan.q[2] = desc.qTZ;
        }
        keyframeCount = std::max(keyframeCount, desc.keyCount2);

        if (desc.keyCount3)
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::Scale;
            chan.i[0] = desc.initSX;
            chan.q[0] = desc.qSX;
            chan.i[1] = desc.initSY;
            chan.q[1] = desc.qSY;
            chan.i[2] = desc.initSZ;
            chan.q[2] = desc.qSZ;
        }
        keyframeCount = std::max(keyframeCount, desc.keyCount3);
    }

    size_t bsSize = DNAANIM::ComputeBitstreamSize(keyframeCount, channels);
    std::unique_ptr<atUint8[]> bsData = reader.readUBytes(bsSize);
    DNAANIM::BitstreamReader bsReader;
    chanKeys = bsReader.read(bsData.get(), keyframeCount, channels, head.rotDiv, head.translationMult, head.scaleMult);
}

void ANIM::ANIM2::write(athena::io::IStreamWriter& writer) const
{
    /* TODO: conform to MP1 ANIM3 */
    Header head;
    head.unk1 = 1;
    head.looping = looping;
    head.interval = mainInterval;
    head.rootBoneId = 0;
    head.scaleMult = 0.f;

    WordBitmap keyBmp;
    size_t frameCount = 0;
    for (atUint32 frame : frames)
    {
        while (keyBmp.getBit(frame))
            ++frame;
        keyBmp.setBit(frame);
        frameCount = frame + 1;
    }
    head.keyBitmapBitCount = keyBmp.getBitCount();
    head.duration = frameCount * mainInterval;
    head.boneChannelCount = bones.size();

    size_t keyframeCount = frames.size();
    std::vector<DNAANIM::Channel> qChannels = channels;
    DNAANIM::BitstreamWriter bsWriter;
    size_t bsSize;
    std::unique_ptr<atUint8[]> bsData = bsWriter.write(chanKeys, keyframeCount, qChannels,
                                                       m_version == 3 ? 0x7fffff : 0x7fff,
                                                       head.rotDiv, head.translationMult, head.scaleMult, bsSize);

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

size_t ANIM::ANIM2::binarySize(size_t __isz) const
{
    Header head;

    WordBitmap keyBmp;
    for (atUint32 frame : frames)
    {
        while (keyBmp.getBit(frame))
            ++frame;
        keyBmp.setBit(frame);
    }

    __isz = head.binarySize(__isz);
    __isz = keyBmp.binarySize(__isz);
    __isz += 4;
    for (const std::pair<atUint32, std::tuple<bool,bool,bool>>& bone : bones)
    {
        __isz += 7;
        if (std::get<0>(bone.second))
            __isz += 9;
        if (std::get<1>(bone.second))
            __isz += 9;
        if (std::get<2>(bone.second))
            __isz += 9;
    }

    return __isz + DNAANIM::ComputeBitstreamSize(frames.size(), channels);
}

}
}
