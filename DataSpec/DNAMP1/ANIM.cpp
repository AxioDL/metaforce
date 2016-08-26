#include "ANIM.hpp"
#include "zeus/CVector3f.hpp"

namespace DataSpec
{
namespace DNAMP1
{

using ANIMOutStream = hecl::BlenderConnection::PyOutStream::ANIMOutStream;

void ANIM::IANIM::sendANIMToBlender(hecl::BlenderConnection::PyOutStream& os, const DNAANIM::RigInverter<CINF>& rig) const
{
    os.format("act.hecl_fps = round(%f)\n", (1.0f / mainInterval));

    auto kit = chanKeys.begin();

    std::vector<zeus::CQuaternion> fixedRotKeys;
    std::vector<zeus::CVector3f> fixedTransKeys;

    for (const std::pair<atUint32, bool>& bone : bones)
    {
        const std::string* bName = rig.getCINF().getBoneNameFromId(bone.first);
        if (!bName)
            continue;

        os.format("bone_string = '%s'\n", bName->c_str());
        os <<     "action_group = act.groups.new(bone_string)\n"
                  "\n"
                  "rotCurves = []\n"
                  "rotCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].rotation_quaternion', index=0, action_group=bone_string))\n"
                  "rotCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].rotation_quaternion', index=1, action_group=bone_string))\n"
                  "rotCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].rotation_quaternion', index=2, action_group=bone_string))\n"
                  "rotCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].rotation_quaternion', index=3, action_group=bone_string))\n"
                  "\n";

        if (bone.second)
            os << "transCurves = []\n"
                  "transCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].location', index=0, action_group=bone_string))\n"
                  "transCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].location', index=1, action_group=bone_string))\n"
                  "transCurves.append(act.fcurves.new('pose.bones[\"'+bone_string+'\"].location', index=2, action_group=bone_string))\n"
                  "\n";

        ANIMOutStream ao = os.beginANIMCurve();

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

        if (bone.second)
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
    channels.clear();
    for (size_t b=0 ; b<boneCount ; ++b)
    {
        bones.emplace_back(boneMap[b], false);
        atUint8 idx = reader.readUByte();
        channels.emplace_back();
        DNAANIM::Channel& chan = channels.back();
        chan.type = DNAANIM::Channel::Type::Rotation;
        if (idx != 0xff)
        {
            bones.back().second = true;
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::Translation;
        }
    }

    reader.readUint32Big();
    chanKeys.clear();
    chanKeys.reserve(channels.size());
    for (const std::pair<atUint32, bool>& bone : bones)
    {
        chanKeys.emplace_back();
        std::vector<DNAANIM::Value>& keys = chanKeys.back();
        for (size_t k=0 ; k<head.keyCount ; ++k)
            keys.emplace_back(reader.readVec4fBig());

        if (bone.second)
            chanKeys.emplace_back();
    }

    reader.readUint32Big();
    auto kit = chanKeys.begin();
    for (const std::pair<atUint32, bool>& bone : bones)
    {
        ++kit;
        if (bone.second)
        {
            std::vector<DNAANIM::Value>& keys = *kit++;
            for (size_t k=0 ; k<head.keyCount ; ++k)
                keys.emplace_back(reader.readVec3fBig());
        }
    }

    evnt.read(reader);
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
    for (const std::pair<atUint32, bool>& bone : bones)
        maxId = std::max(maxId, bone.first);
    head.boneSlotCount = maxId + 1;
    head.write(writer);

    for (size_t s=0 ; s<head.boneSlotCount ; ++s)
    {
        size_t boneIdx = 0;
        bool found = false;
        for (const std::pair<atUint32, bool>& bone : bones)
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
    for (const std::pair<atUint32, bool>& bone : bones)
    {
        if (bone.second)
            writer.writeUByte(boneIdx);
        else
            writer.writeUByte(0xff);
        ++boneIdx;
    }

    writer.writeUint32Big(bones.size() * head.keyCount);
    auto cit = chanKeys.begin();
    atUint32 transKeyCount = 0;
    for (const std::pair<atUint32, bool>& bone : bones)
    {
        const std::vector<DNAANIM::Value>& keys = *cit++;
        auto kit = keys.begin();
        for (size_t k=0 ; k<head.keyCount ; ++k)
            writer.writeVec4fBig((*kit++).v4);
        if (bone.second)
        {
            transKeyCount += head.keyCount;
            ++cit;
        }
    }

    writer.writeUint32Big(transKeyCount);
    cit = chanKeys.begin();
    for (const std::pair<atUint32, bool>& bone : bones)
    {
        ++cit;
        if (bone.second)
        {
            const std::vector<DNAANIM::Value>& keys = *cit++;
            auto kit = keys.begin();
            for (size_t k=0 ; k<head.keyCount ; ++k)
                writer.writeVec3fBig((*kit++).v3);
        }
    }

    evnt.write(writer);
}

size_t ANIM::ANIM0::binarySize(size_t __isz) const
{
    Header head;

    atUint32 maxId = 0;
    for (const std::pair<atUint32, bool>& bone : bones)
        maxId = std::max(maxId, bone.first);

    __isz = head.binarySize(__isz);
    __isz += maxId + 1;
    __isz += bones.size() + 4;

    __isz += 8;
    for (const std::pair<atUint32, bool>& bone : bones)
    {
        __isz += head.keyCount * 16;
        if (bone.second)
            __isz += head.keyCount * 12;
    }

    return __isz + 4;
}

void ANIM::ANIM2::read(athena::io::IStreamReader& reader)
{
    Header head;
    head.read(reader);
    evnt = head.evnt;
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
    reader.seek(8);

    bones.clear();
    bones.reserve(head.boneChannelCount);
    channels.clear();
    channels.reserve(head.boneChannelCount);
    atUint16 keyframeCount = 0;
    for (size_t b=0 ; b<head.boneChannelCount ; ++b)
    {
        ChannelDesc desc;
        desc.read(reader);
        bones.emplace_back(desc.id, desc.keyCount2 != 0);

        if (desc.keyCount1)
        {
            channels.emplace_back();
            DNAANIM::Channel& chan = channels.back();
            chan.type = DNAANIM::Channel::Type::Rotation;
            chan.id = desc.id;
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
            chan.id = desc.id;
            chan.i[0] = desc.initTX;
            chan.q[0] = desc.qTX;
            chan.i[1] = desc.initTY;
            chan.q[1] = desc.qTY;
            chan.i[2] = desc.initTZ;
            chan.q[2] = desc.qTZ;
        }
    }

    size_t bsSize = DNAANIM::ComputeBitstreamSize(keyframeCount, channels);
    std::unique_ptr<atUint8[]> bsData = reader.readUBytes(bsSize);
    DNAANIM::BitstreamReader bsReader;
    chanKeys = bsReader.read(bsData.get(), keyframeCount, channels, head.rotDiv, head.translationMult, 0.f);
}

void ANIM::ANIM2::write(athena::io::IStreamWriter& writer) const
{
    Header head;
    head.evnt = evnt;
    head.unk0 = 1;
    head.interval = mainInterval;
    head.rootBoneId = 3;
    head.unk2 = 0;
    head.unk3 = 1;

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
    float scaleMult;
    std::unique_ptr<atUint8[]> bsData = bsWriter.write(chanKeys, keyframeCount, qChannels,
                                                       m_version == 3 ? 0x7fffff : 0x7fff,
                                                       head.rotDiv, head.translationMult, scaleMult, bsSize);

    /* TODO: Figure out proper scratch size computation */
    head.scratchSize = keyframeCount * channels.size() * 16;

    head.write(writer);
    keyBmp.write(writer);
    writer.writeUint32Big(head.boneChannelCount);
    writer.writeUint32Big(head.boneChannelCount);
    auto cit = qChannels.begin();
    for (const std::pair<atUint32, bool>& bone : bones)
    {
        ChannelDesc desc;
        desc.id = bone.first;
        DNAANIM::Channel& chan = *cit++;
        desc.keyCount1 = keyframeCount;
        desc.initRX = chan.i[0];
        desc.qRX = chan.q[0];
        desc.initRY = chan.i[1];
        desc.qRY = chan.q[1];
        desc.initRZ = chan.i[2];
        desc.qRZ = chan.q[2];
        if (bone.second)
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
        desc.write(writer);
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
    __isz += 8;
    for (const std::pair<atUint32, bool>& bone : bones)
    {
        __isz += 17;
        if (bone.second)
            __isz += 9;
    }

    return __isz + DNAANIM::ComputeBitstreamSize(frames.size(), channels);
}

ANIM::ANIM(const BlenderAction& act,
           const std::unordered_map<std::string, atInt32>& idMap,
           const DNAANIM::RigInverter<CINF>& rig,
           bool pc)
{
    m_anim.reset(new struct ANIM2(pc));
    IANIM& newAnim = *m_anim;

    newAnim.bones.reserve(act.channels.size());
    size_t extChanCount = 0;
    for (const BlenderAction::Channel& chan : act.channels)
    {
        auto search = idMap.find(chan.boneName);
        if (search == idMap.cend())
        {
            Log.report(logvisor::Warning, "unable to find id for bone '%s'", chan.boneName.c_str());
            continue;
        }

        extChanCount += std::max(zeus::PopCount(chan.attrMask), 2);
        newAnim.bones.emplace_back(search->second, (chan.attrMask & 0x2) != 0);
    }

    newAnim.frames.reserve(act.frames.size());
    for (int32_t frame : act.frames)
        newAnim.frames.push_back(frame);

    newAnim.channels.reserve(extChanCount);
    newAnim.chanKeys.reserve(extChanCount);

    for (const BlenderAction::Channel& chan : act.channels)
    {
        auto search = idMap.find(chan.boneName);
        if (search == idMap.cend())
            continue;

        newAnim.channels.emplace_back();
        DNAANIM::Channel& newChan = newAnim.channels.back();
        newChan.type = DNAANIM::Channel::Type::Rotation;
        newChan.id = search->second;

        newAnim.chanKeys.emplace_back();
        std::vector<DNAANIM::Value>& rotVals = newAnim.chanKeys.back();
        rotVals.reserve(chan.keys.size());
        for (const BlenderAction::Channel::Key& key : chan.keys)
        {
            zeus::CQuaternion q(key.rotation.val);
            q = rig.restoreRotation(newChan.id, q);
            rotVals.emplace_back(q.w, q.x, q.y, q.z);
        }

        if (chan.attrMask & 0x2)
        {
            newAnim.channels.emplace_back();
            DNAANIM::Channel& newChan = newAnim.channels.back();
            newChan.type = DNAANIM::Channel::Type::Translation;
            newChan.id = search->second;

            newAnim.chanKeys.emplace_back();
            std::vector<DNAANIM::Value>& transVals = newAnim.chanKeys.back();
            transVals.reserve(chan.keys.size());
            for (const BlenderAction::Channel::Key& key : chan.keys)
            {
                zeus::CVector3f pos(key.position.val);
                pos = rig.restorePosition(newChan.id, pos, true);
                transVals.emplace_back(pos.x, pos.y, pos.z);
            }
        }
    }

    newAnim.mainInterval = act.interval;
}

}
}
