#include "ANCS.hpp"

namespace Retro
{
namespace DNAMP1
{

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::read(Athena::io::IStreamReader& reader)
{
    parmType = reader.readUint32();
    unk1 = reader.readUint32();
    unk2 = reader.readFloat();
    switch(DataType(parmType))
    {
    case DataType::DTInt32:
        parmVals[0].int32 = reader.readInt32();
        parmVals[1].int32 = reader.readInt32();
        break;
    case DataType::DTUInt32:
    case DataType::DTEnum:
        parmVals[0].uint32 = reader.readUint32();
        parmVals[1].uint32 = reader.readUint32();
        break;
    case DataType::DTFloat:
        parmVals[0].float32 = reader.readFloat();
        parmVals[1].float32 = reader.readFloat();
        break;
    case DataType::DTBool:
        parmVals[0].bool1 = reader.readBool();
        parmVals[1].bool1 = reader.readBool();
        break;
    }
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::write(Athena::io::IStreamWriter& writer) const
{
    writer.writeUint32(parmType);
    writer.writeUint32(unk1);
    writer.writeFloat(unk2);
    switch(DataType(parmType))
    {
    case DataType::DTInt32:
        writer.writeInt32(parmVals[0].int32);
        writer.writeInt32(parmVals[1].int32);
        break;
    case DataType::DTUInt32:
    case DataType::DTEnum:
        writer.writeUint32(parmVals[0].uint32);
        writer.writeUint32(parmVals[0].uint32);
        break;
    case DataType::DTFloat:
        writer.writeFloat(parmVals[0].float32);
        writer.writeFloat(parmVals[0].float32);
        break;
    case DataType::DTBool:
        writer.writeBool(parmVals[0].bool1);
        writer.writeBool(parmVals[0].bool1);
        break;
    }
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::fromYAML(Athena::io::YAMLDocReader& reader)
{
    parmType = reader.readUint32("parmType");
    unk1 = reader.readUint32("unk1");
    unk2 = reader.readFloat("unk2");
    reader.enterSubVector("parmVals");
    switch(DataType(parmType))
    {
    case DataType::DTInt32:
        parmVals[0].int32 = reader.readInt32(nullptr);
        parmVals[1].int32 = reader.readInt32(nullptr);
        break;
    case DataType::DTUInt32:
    case DataType::DTEnum:
        parmVals[0].uint32 = reader.readUint32(nullptr);
        parmVals[1].uint32 = reader.readUint32(nullptr);
        break;
    case DataType::DTFloat:
        parmVals[0].float32 = reader.readFloat(nullptr);
        parmVals[1].float32 = reader.readFloat(nullptr);
        break;
    case DataType::DTBool:
        parmVals[0].bool1 = reader.readBool(nullptr);
        parmVals[1].bool1 = reader.readBool(nullptr);
        break;
    default: break;
    }
    reader.leaveSubVector();
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    writer.writeUint32("parmType", parmType);
    writer.writeUint32("unk1", unk1);
    writer.writeFloat("unk2", unk2);
    writer.enterSubVector("parmVals");
    switch(DataType(parmType))
    {
    case DataType::DTInt32:
        writer.writeInt32(nullptr, parmVals[0].int32);
        writer.writeInt32(nullptr, parmVals[1].int32);
        break;
    case DataType::DTUInt32:
    case DataType::DTEnum:
        writer.writeUint32(nullptr, parmVals[0].uint32);
        writer.writeUint32(nullptr, parmVals[0].uint32);
        break;
    case DataType::DTFloat:
        writer.writeFloat(nullptr, parmVals[0].float32);
        writer.writeFloat(nullptr, parmVals[0].float32);
        break;
    case DataType::DTBool:
        writer.writeBool(nullptr, parmVals[0].bool1);
        writer.writeBool(nullptr, parmVals[0].bool1);
        break;
    }
    writer.leaveSubVector();
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::read(Athena::io::IStreamReader& reader)
{
    id = reader.readUint32();
    atUint32 parmInfoCount = reader.readUint32();
    atUint32 animInfoCount = reader.readUint32();

    parmInfos.reserve(parmInfoCount);
    for (int i=0 ; i<parmInfoCount ; ++i)
    {
        parmInfos.emplace_back();
        parmInfos.back().read(reader);
    }

    animInfos.reserve(animInfoCount);
    for (int i=0 ; i<animInfoCount ; ++i)
    {
        animInfos.emplace_back();
        AnimInfo& ai = animInfos.back();
        ai.id = reader.readUint32();
        ai.parmVals.reserve(parmInfoCount);
        for (const ParmInfo& pi : parmInfos)
        {
            switch(ParmInfo::DataType(pi.parmType))
            {
            case ParmInfo::DTInt32:
                ai.parmVals.emplace_back(reader.readInt32());
                break;
            case ParmInfo::DTUInt32:
            case ParmInfo::DTEnum:
                ai.parmVals.emplace_back(reader.readUint32());
                break;
            case ParmInfo::DTFloat:
                ai.parmVals.emplace_back(reader.readFloat());
                break;
            case ParmInfo::DTBool:
                ai.parmVals.emplace_back(reader.readBool());
                break;
            default: break;
            }
        }
    }
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::write(Athena::io::IStreamWriter& writer) const
{
    writer.writeUint32(id);
    writer.writeUint32(parmInfos.size());
    writer.writeUint32(animInfos.size());

    for (const ParmInfo& pi : parmInfos)
        pi.write(writer);

    for (const AnimInfo& ai : animInfos)
    {
        writer.writeUint32(ai.id);
        auto it = ai.parmVals.begin();
        for (const ParmInfo& pi : parmInfos)
        {
            ParmInfo::Parm pVal;
            if (it != ai.parmVals.end())
                pVal = *it++;
            switch(ParmInfo::DataType(pi.parmType))
            {
            case ParmInfo::DTInt32:
                writer.writeInt32(pVal.int32);
                break;
            case ParmInfo::DTUInt32:
            case ParmInfo::DTEnum:
                writer.writeUint32(pVal.uint32);
                break;
            case ParmInfo::DTFloat:
                writer.writeFloat(pVal.float32);
                break;
            case ParmInfo::DTBool:
                writer.writeBool(pVal.bool1);
                break;
            default: break;
            }
        }
    }
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::fromYAML(Athena::io::YAMLDocReader& reader)
{
    id = reader.readUint32("id");
    atUint32 parmInfoCount = reader.readUint32("parmInfoCount");
    atUint32 animInfoCount = reader.readUint32("animInfoCount");

    parmInfos.reserve(parmInfoCount);
    reader.enterSubVector("parmInfos");
    for (int i=0 ; i<parmInfoCount ; ++i)
    {
        parmInfos.emplace_back();
        parmInfos.back().fromYAML(reader);
    }
    reader.leaveSubVector();

    animInfos.reserve(animInfoCount);
    reader.enterSubVector("animInfos");
    for (int i=0 ; i<animInfoCount ; ++i)
    {
        animInfos.emplace_back();
        AnimInfo& ai = animInfos.back();
        reader.enterSubRecord(nullptr);
        ai.id = reader.readUint32("id");
        ai.parmVals.reserve(parmInfoCount);
        reader.enterSubVector("parmVals");
        for (const ParmInfo& pi : parmInfos)
        {
            switch(ParmInfo::DataType(pi.parmType))
            {
            case ParmInfo::DTInt32:
                ai.parmVals.emplace_back(reader.readInt32(nullptr));
                break;
            case ParmInfo::DTUInt32:
            case ParmInfo::DTEnum:
                ai.parmVals.emplace_back(reader.readUint32(nullptr));
                break;
            case ParmInfo::DTFloat:
                ai.parmVals.emplace_back(reader.readFloat(nullptr));
                break;
            case ParmInfo::DTBool:
                ai.parmVals.emplace_back(reader.readBool(nullptr));
                break;
            default: break;
            }
        }
        reader.leaveSubVector();
        reader.leaveSubRecord();
    }
    reader.leaveSubVector();
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    writer.writeUint32("id", id);
    writer.writeUint32("parmInfoCount", parmInfos.size());
    writer.writeUint32("animInfoCount", animInfos.size());

    writer.enterSubVector("parmInfos");
    for (const ParmInfo& pi : parmInfos)
    {
        writer.enterSubRecord(nullptr);
        pi.toYAML(writer);
        writer.leaveSubRecord();
    }
    writer.leaveSubVector();

    writer.enterSubVector("animInfos");
    for (const AnimInfo& ai : animInfos)
    {
        writer.enterSubRecord("info");
        writer.writeUint32("id", ai.id);
        auto it = ai.parmVals.begin();
        writer.enterSubVector("parms");
        for (const ParmInfo& pi : parmInfos)
        {
            ParmInfo::Parm pVal;
            if (it != ai.parmVals.end())
                pVal = *it++;
            switch(ParmInfo::DataType(pi.parmType))
            {
            case ParmInfo::DTInt32:
                writer.writeInt32(nullptr, pVal.int32);
                break;
            case ParmInfo::DTUInt32:
            case ParmInfo::DTEnum:
                writer.writeUint32(nullptr, pVal.uint32);
                break;
            case ParmInfo::DTFloat:
                writer.writeFloat(nullptr, pVal.float32);
                break;
            case ParmInfo::DTBool:
                writer.writeBool(nullptr, pVal.bool1);
                break;
            default: break;
            }
        }
        writer.leaveSubVector();
        writer.leaveSubRecord();
    }
    writer.leaveSubVector();
}

void ANCS::CharacterSet::CharacterInfo::read(Athena::io::IStreamReader& reader)
{
    idx = reader.readUint32();
    atUint16 sectionCount = reader.readUint16();
    name = reader.readString();
    cmdl.read(reader);
    cskr.read(reader);
    cinf.read(reader);

    atUint32 animationCount = reader.readUint32();
    animations.reserve(animationCount);
    for (int i=0 ; i<animationCount ; ++i)
    {
        animations.emplace_back();
        animations.back().read(reader);
    }

    pasDatabase.read(reader);

    atUint32 partCount = reader.readUint32();
    partResData.part.clear();
    partResData.part.reserve(partCount);
    for (int i=0 ; i<partCount ; ++i)
    {
        partResData.part.emplace_back();
        partResData.part.back().read(reader);
    }

    atUint32 swhcCount = reader.readUint32();
    partResData.swhc.clear();
    partResData.swhc.reserve(swhcCount);
    for (int i=0 ; i<swhcCount ; ++i)
    {
        partResData.swhc.emplace_back();
        partResData.swhc.back().read(reader);
    }

    atUint32 unkCount = reader.readUint32();
    partResData.unk.clear();
    partResData.unk.reserve(unkCount);
    for (int i=0 ; i<unkCount ; ++i)
    {
        partResData.unk.emplace_back();
        partResData.unk.back().read(reader);
    }

    if (sectionCount > 5)
    {
        atUint32 elscCount = reader.readUint32();
        partResData.elsc.clear();
        partResData.elsc.reserve(elscCount);
        for (int i=0 ; i<elscCount ; ++i)
        {
            partResData.elsc.emplace_back();
            partResData.elsc.back().read(reader);
        }
    }

    unk1 = reader.readUint32();

    animAABBs.clear();
    if (sectionCount > 1)
    {
        atUint32 aabbCount = reader.readUint32();
        animAABBs.reserve(aabbCount);
        for (int i=0 ; i<aabbCount ; ++i)
        {
            animAABBs.emplace_back();
            animAABBs.back().read(reader);
        }
    }

    effects.clear();
    if (sectionCount > 2)
    {
        atUint32 effectCount = reader.readUint32();
        effects.reserve(effectCount);
        for (int i=0 ; i<effectCount ; ++i)
        {
            effects.emplace_back();
            effects.back().read(reader);
        }
    }

    if (sectionCount > 3)
    {
        cmdlOverride.read(reader);
        cskrOverride.read(reader);
    }

    animIdxs.clear();
    if (sectionCount > 4)
    {
        atUint32 aidxCount = reader.readUint32();
        animIdxs.reserve(aidxCount);
        for (int i=0 ; i<aidxCount ; ++i)
            animIdxs.emplace_back(reader.readUint32());
    }
}

void ANCS::CharacterSet::CharacterInfo::write(Athena::io::IStreamWriter& writer) const
{
    writer.writeUint32(idx);

    atUint16 sectionCount;
    if (partResData.elsc.size())
        sectionCount = 6;
    else if (animIdxs.size())
        sectionCount = 5;
    else if (cmdlOverride)
        sectionCount = 4;
    else if (effects.size())
        sectionCount = 3;
    else if (animAABBs.size())
        sectionCount = 2;
    else
        sectionCount = 1;
    writer.writeUint16(sectionCount);

    writer.writeString(name);
    cmdl.write(writer);
    cskr.write(writer);
    cinf.write(writer);

    writer.writeUint32(animations.size());
    for (const Animation& anim : animations)
        anim.write(writer);

    pasDatabase.write(writer);

    writer.writeUint32(partResData.part.size());
    for (const UniqueID32& id : partResData.part)
        id.write(writer);

    writer.writeUint32(partResData.swhc.size());
    for (const UniqueID32& id : partResData.swhc)
        id.write(writer);

    writer.writeUint32(partResData.unk.size());
    for (const UniqueID32& id : partResData.unk)
        id.write(writer);

    if (sectionCount > 5)
    {
        writer.writeUint32(partResData.elsc.size());
        for (const UniqueID32& id : partResData.elsc)
            id.write(writer);
    }

    writer.writeUint32(unk1);

    if (sectionCount > 1)
    {
        writer.writeUint32(animAABBs.size());
        for (const ActionAABB& aabb : animAABBs)
            aabb.write(writer);
    }

    if (sectionCount > 2)
    {
        writer.writeUint32(effects.size());
        for (const Effect& effect : effects)
            effect.write(writer);
    }

    if (sectionCount > 3)
    {
        cmdlOverride.write(writer);
        cskrOverride.write(writer);
    }

    if (sectionCount > 4)
    {
        writer.writeUint32(animIdxs.size());
        for (atUint32 idx : animIdxs)
            writer.writeUint32(idx);
    }
}

void ANCS::CharacterSet::CharacterInfo::fromYAML(Athena::io::YAMLDocReader& reader)
{
    idx = reader.readUint32("idx");
    atUint16 sectionCount = reader.readUint16("sectionCount");
    name = reader.readString("name");
    reader.enterSubRecord("cmdl");
    cmdl.fromYAML(reader);
    reader.leaveSubRecord();
    reader.enterSubRecord("cskr");
    cskr.fromYAML(reader);
    reader.leaveSubRecord();
    reader.enterSubRecord("cinf");
    cinf.fromYAML(reader);
    reader.leaveSubRecord();

    atUint32 animationCount = reader.readUint32("animationCount");
    animations.reserve(animationCount);
    reader.enterSubVector("animations");
    for (int i=0 ; i<animationCount ; ++i)
    {
        animations.emplace_back();
        reader.enterSubRecord(nullptr);
        animations.back().fromYAML(reader);
        reader.leaveSubRecord();
    }
    reader.leaveSubVector();

    reader.enterSubRecord("pasDatabase");
    pasDatabase.fromYAML(reader);
    reader.leaveSubRecord();

    atUint32 partCount = reader.readUint32("partCount");
    partResData.part.clear();
    partResData.part.reserve(partCount);
    reader.enterSubVector("part");
    for (int i=0 ; i<partCount ; ++i)
    {
        partResData.part.emplace_back();
        reader.enterSubRecord(nullptr);
        partResData.part.back().fromYAML(reader);
        reader.leaveSubRecord();
    }
    reader.leaveSubVector();

    atUint32 swhcCount = reader.readUint32("swhcCount");
    partResData.swhc.clear();
    partResData.swhc.reserve(swhcCount);
    reader.enterSubVector("swhc");
    for (int i=0 ; i<swhcCount ; ++i)
    {
        partResData.swhc.emplace_back();
        reader.enterSubRecord(nullptr);
        partResData.swhc.back().fromYAML(reader);
        reader.leaveSubRecord();
    }
    reader.leaveSubVector();

    atUint32 unkCount = reader.readUint32("unkCount");
    partResData.unk.clear();
    partResData.unk.reserve(unkCount);
    reader.enterSubVector("unk");
    for (int i=0 ; i<unkCount ; ++i)
    {
        partResData.unk.emplace_back();
        reader.enterSubRecord(nullptr);
        partResData.unk.back().fromYAML(reader);
        reader.leaveSubRecord();
    }
    reader.leaveSubVector();

    if (sectionCount > 5)
    {
        atUint32 elscCount = reader.readUint32("elscCount");
        partResData.elsc.clear();
        partResData.elsc.reserve(elscCount);
        reader.enterSubVector("elsc");
        for (int i=0 ; i<elscCount ; ++i)
        {
            partResData.elsc.emplace_back();
            reader.enterSubRecord(nullptr);
            partResData.elsc.back().fromYAML(reader);
            reader.leaveSubRecord();
        }
        reader.leaveSubVector();
    }

    unk1 = reader.readUint32("unk1");

    animAABBs.clear();
    if (sectionCount > 1)
    {
        atUint32 aabbCount = reader.readUint32("animAABBCount");
        animAABBs.reserve(aabbCount);
        reader.enterSubVector("animAABBs");
        for (int i=0 ; i<aabbCount ; ++i)
        {
            animAABBs.emplace_back();
            reader.enterSubRecord(nullptr);
            animAABBs.back().fromYAML(reader);
            reader.leaveSubRecord();
        }
        reader.leaveSubVector();
    }

    effects.clear();
    if (sectionCount > 2)
    {
        atUint32 effectCount = reader.readUint32("effectCount");
        effects.reserve(effectCount);
        reader.enterSubVector("effects");
        for (int i=0 ; i<effectCount ; ++i)
        {
            effects.emplace_back();
            reader.enterSubRecord(nullptr);
            effects.back().fromYAML(reader);
            reader.leaveSubRecord();
        }
        reader.leaveSubVector();
    }

    if (sectionCount > 3)
    {
        reader.enterSubRecord("cmdlOverride");
        cmdlOverride.fromYAML(reader);
        reader.leaveSubRecord();
        reader.enterSubRecord("cskrOverride");
        cskrOverride.fromYAML(reader);
        reader.leaveSubRecord();
    }

    animIdxs.clear();
    if (sectionCount > 4)
    {
        atUint32 animIdxCount = reader.readUint32("animIdxCount");
        animIdxs.reserve(animIdxCount);
        reader.enterSubVector("animIdxs");
        for (int i=0 ; i<animIdxCount ; ++i)
            animIdxs.emplace_back(reader.readUint32(nullptr));
        reader.leaveSubVector();
    }
}

void ANCS::CharacterSet::CharacterInfo::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    writer.writeUint32("idx", idx);

    atUint16 sectionCount;
    if (partResData.elsc.size())
        sectionCount = 6;
    else if (animIdxs.size())
        sectionCount = 5;
    else if (cmdlOverride)
        sectionCount = 4;
    else if (effects.size())
        sectionCount = 3;
    else if (animAABBs.size())
        sectionCount = 2;
    else
        sectionCount = 1;
    writer.writeUint16("sectionCount", sectionCount);

    writer.writeString("name", name);
    writer.enterSubRecord("cmdl");
    cmdl.toYAML(writer);
    writer.leaveSubRecord();
    writer.enterSubRecord("cskr");
    cskr.toYAML(writer);
    writer.leaveSubRecord();
    writer.enterSubRecord("cinf");
    cinf.toYAML(writer);
    writer.leaveSubRecord();

    writer.writeUint32("animationCount", animations.size());
    writer.enterSubVector("animations");
    for (const Animation& anim : animations)
    {
        writer.enterSubRecord(nullptr);
        anim.toYAML(writer);
        writer.leaveSubRecord();
    }
    writer.leaveSubVector();

    writer.enterSubRecord("pasDatabase");
    pasDatabase.toYAML(writer);
    writer.leaveSubRecord();

    writer.writeUint32("partCount", partResData.part.size());
    writer.enterSubVector("part");
    for (const UniqueID32& id : partResData.part)
    {
        writer.enterSubRecord(nullptr);
        id.toYAML(writer);
        writer.leaveSubRecord();
    }
    writer.leaveSubVector();

    writer.writeUint32("swhcCount", partResData.swhc.size());
    writer.enterSubVector("swhc");
    for (const UniqueID32& id : partResData.swhc)
    {
        writer.enterSubRecord(nullptr);
        id.toYAML(writer);
        writer.leaveSubRecord();
    }
    writer.leaveSubVector();

    writer.writeUint32("unkCount", partResData.unk.size());
    writer.enterSubVector("unk");
    for (const UniqueID32& id : partResData.unk)
    {
        writer.enterSubRecord(nullptr);
        id.toYAML(writer);
        writer.leaveSubRecord();
    }
    writer.leaveSubVector();

    if (sectionCount > 5)
    {
        writer.writeUint32("elscCount", partResData.elsc.size());
        writer.enterSubVector("elsc");
        for (const UniqueID32& id : partResData.elsc)
        {
            writer.enterSubRecord(nullptr);
            id.toYAML(writer);
            writer.leaveSubRecord();
        }
        writer.leaveSubVector();
    }

    writer.writeUint32("unk1", unk1);

    if (sectionCount > 1)
    {
        writer.writeUint32("animAABBCount", animAABBs.size());
        writer.enterSubVector("animAABBs");
        for (const ActionAABB& aabb : animAABBs)
        {
            writer.enterSubRecord(nullptr);
            aabb.toYAML(writer);
            writer.leaveSubRecord();
        }
        writer.leaveSubVector();
    }

    if (sectionCount > 2)
    {
        writer.writeUint32("effectCount", effects.size());
        writer.enterSubVector("effects");
        for (const Effect& effect : effects)
        {
            writer.enterSubRecord(nullptr);
            effect.toYAML(writer);
            writer.leaveSubRecord();
        }
        writer.leaveSubVector();
    }

    if (sectionCount > 3)
    {
        writer.enterSubRecord("cmdlOverride");
        cmdlOverride.toYAML(writer);
        writer.leaveSubRecord();
        writer.enterSubRecord("cskrOverride");
        cskrOverride.toYAML(writer);
        writer.leaveSubRecord();
    }

    if (sectionCount > 4)
    {
        writer.writeUint32("animIdxCount", animIdxs.size());
        writer.enterSubVector("animIdxs");
        for (atUint32 idx : animIdxs)
            writer.writeUint32(nullptr, idx);
        writer.leaveSubVector();
    }
}

void ANCS::AnimationSet::MetaAnimFactory::read(Athena::io::IStreamReader& reader)
{
    IMetaAnim::Type type(IMetaAnim::Type(reader.readUint32()));
    switch (type)
    {
    case IMetaAnim::MAPrimitive:
        m_anim.reset(new struct MetaAnimPrimitive);
        m_anim->read(reader);
        break;
    case IMetaAnim::MABlend:
        m_anim.reset(new struct MetaAnimBlend);
        m_anim->read(reader);
        break;
    case IMetaAnim::MAPhaseBlend:
        m_anim.reset(new struct MetaAnimPhaseBlend);
        m_anim->read(reader);
        break;
    case IMetaAnim::MARandom:
        m_anim.reset(new struct MetaAnimRandom);
        m_anim->read(reader);
        break;
    case IMetaAnim::MASequence:
        m_anim.reset(new struct MetaAnimSequence);
        m_anim->read(reader);
        break;
    default:
        m_anim.reset(nullptr);
        break;
    }
}

void ANCS::AnimationSet::MetaAnimFactory::write(Athena::io::IStreamWriter& writer) const
{
    if (!m_anim)
        return;
    writer.writeInt32(m_anim->m_type);
    m_anim->write(writer);
}

void ANCS::AnimationSet::MetaAnimFactory::fromYAML(Athena::io::YAMLDocReader& reader)
{
    std::string type = reader.readString("type");
    std::transform(type.begin(), type.end(), type.begin(), tolower);
    if (!type.compare("primitive"))
    {
        m_anim.reset(new struct MetaAnimPrimitive);
        m_anim->fromYAML(reader);
    }
    else if (!type.compare("blend"))
    {
        m_anim.reset(new struct MetaAnimBlend);
        m_anim->fromYAML(reader);
    }
    else if (!type.compare("phaseblend"))
    {
        m_anim.reset(new struct MetaAnimPhaseBlend);
        m_anim->fromYAML(reader);
    }
    else if (!type.compare("random"))
    {
        m_anim.reset(new struct MetaAnimRandom);
        m_anim->fromYAML(reader);
    }
    else if (!type.compare("sequence"))
    {
        m_anim.reset(new struct MetaAnimSequence);
        m_anim->fromYAML(reader);
    }
    else
    {
        m_anim.reset(nullptr);
    }

}

void ANCS::AnimationSet::MetaTransFactory::read(Athena::io::IStreamReader& reader)
{
    IMetaTrans::Type type(IMetaTrans::Type(reader.readUint32()));
    switch (type)
    {
    case IMetaTrans::MTMetaAnim:
        m_trans.reset(new struct MetaTransMetaAnim);
        m_trans->read(reader);
        break;
    case IMetaTrans::MTTrans:
        m_trans.reset(new struct MetaTransTrans);
        m_trans->read(reader);
        break;
    case IMetaTrans::MTPhaseTrans:
        m_trans.reset(new struct MetaTransPhaseTrans);
        m_trans->read(reader);
        break;
    case IMetaTrans::MTNoTrans:
    default:
        m_trans.reset(nullptr);
        break;
    }
}

void ANCS::AnimationSet::MetaTransFactory::write(Athena::io::IStreamWriter& writer) const
{
    if (!m_trans)
    {
        writer.writeInt32(IMetaTrans::MTNoTrans);
        return;
    }
    writer.writeInt32(m_trans->m_type);
    m_trans->write(writer);
}

void ANCS::AnimationSet::MetaTransFactory::fromYAML(Athena::io::YAMLDocReader& reader)
{
    std::string type = reader.readString("type");
    std::transform(type.begin(), type.end(), type.begin(), tolower);
    if (!type.compare("metaanim"))
    {
        m_trans.reset(new struct MetaTransMetaAnim);
        m_trans->fromYAML(reader);
    }
    else if (!type.compare("trans"))
    {
        m_trans.reset(new struct MetaTransTrans);
        m_trans->fromYAML(reader);
    }
    else if (!type.compare("phasetrans"))
    {
        m_trans.reset(new struct MetaTransPhaseTrans);
        m_trans->fromYAML(reader);
    }
    else
    {
        m_trans.reset(nullptr);
    }

}

void ANCS::AnimationSet::MetaTransFactory::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    if (!m_trans)
    {
        writer.writeString("type", "NoTrans");
        return;
    }
    writer.writeString("type", m_trans->m_typeStr?m_trans->m_typeStr:"NoTrans");
    m_trans->toYAML(writer);
}

void ANCS::AnimationSet::MetaAnimFactory::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    if (!m_anim)
        return;
    writer.writeString("type", m_anim->m_typeStr);
    m_anim->toYAML(writer);
}

void ANCS::AnimationSet::read(Athena::io::IStreamReader& reader)
{
    atUint16 sectionCount = reader.readUint16();

    atUint32 animationCount = reader.readUint32();
    animations.reserve(animationCount);
    for (int i=0 ; i<animationCount ; ++i)
    {
        animations.emplace_back();
        animations.back().read(reader);
    }

    atUint32 transitionCount = reader.readUint32();
    transitions.reserve(transitionCount);
    for (int i=0 ; i<transitionCount ; ++i)
    {
        transitions.emplace_back();
        transitions.back().read(reader);
    }

    if (sectionCount > 1)
    {
        atUint32 additiveAnimCount = reader.readUint32();
        additiveAnims.reserve(additiveAnimCount);
        for (int i=0 ; i<additiveAnimCount ; ++i)
        {
            additiveAnims.emplace_back();
            additiveAnims.back().read(reader);
        }
        floatA = reader.readFloat();
        floatB = reader.readFloat();
    }

    if (sectionCount > 2)
    {
        atUint32 halfTransitionCount = reader.readUint32();
        halfTransitions.reserve(halfTransitionCount);
        for (int i=0 ; i<halfTransitionCount ; ++i)
        {
            halfTransitions.emplace_back();
            halfTransitions.back().read(reader);
        }
    }

    if (sectionCount > 3)
    {
        atUint32 animResourcesCount = reader.readUint32();
        animResources.reserve(animResourcesCount);
        for (int i=0 ; i<animResourcesCount ; ++i)
        {
            animResources.emplace_back();
            animResources.back().read(reader);
        }
    }
}

void ANCS::AnimationSet::write(Athena::io::IStreamWriter& writer) const
{
    atUint16 sectionCount;
    if (animResources.size())
        sectionCount = 4;
    else if (halfTransitions.size())
        sectionCount = 3;
    else if (additiveAnims.size())
        sectionCount = 2;
    else
        sectionCount = 1;

    writer.writeUint16(sectionCount);

    writer.writeUint32(animations.size());
    for (const auto& anim : animations)
        anim.write(writer);

    writer.writeUint32(transitions.size());
    for (const auto& trans : transitions)
        trans.write(writer);

    if (sectionCount > 1)
    {
        writer.writeUint32(additiveAnims.size());
        for (const auto& aa : additiveAnims)
            aa.write(writer);
        writer.writeFloat(floatA);
        writer.writeFloat(floatB);
    }

    if (sectionCount > 2)
    {
        writer.writeUint32(halfTransitions.size());
        for (const auto& ht : halfTransitions)
            ht.write(writer);
    }

    if (sectionCount > 3)
    {
        writer.writeUint32(animResources.size());
        for (const auto& ar : animResources)
            ar.write(writer);
    }
}

void ANCS::AnimationSet::fromYAML(Athena::io::YAMLDocReader& reader)
{
    atUint16 sectionCount = reader.readUint16("sectionCount");

    atUint32 animationCount = reader.readUint32("animationCount");
    animations.reserve(animationCount);
    reader.enterSubVector("animations");
    for (int i=0 ; i<animationCount ; ++i)
    {
        animations.emplace_back();
        reader.enterSubRecord(nullptr);
        animations.back().fromYAML(reader);
        reader.leaveSubRecord();
    }
    reader.leaveSubVector();

    atUint32 transitionCount = reader.readUint32("transitionCount");
    transitions.reserve(transitionCount);
    reader.enterSubVector("transitions");
    for (int i=0 ; i<transitionCount ; ++i)
    {
        transitions.emplace_back();
        reader.enterSubRecord(nullptr);
        transitions.back().fromYAML(reader);
        reader.leaveSubRecord();
    }
    reader.leaveSubVector();

    if (sectionCount > 1)
    {
        atUint32 additiveAnimCount = reader.readUint32("additiveAnimCount");
        additiveAnims.reserve(additiveAnimCount);
        reader.enterSubVector("additiveAnims");
        for (int i=0 ; i<additiveAnimCount ; ++i)
        {
            additiveAnims.emplace_back();
            reader.enterSubRecord(nullptr);
            additiveAnims.back().fromYAML(reader);
            reader.leaveSubRecord();
        }
        reader.leaveSubVector();
        floatA = reader.readFloat("floatA");
        floatB = reader.readFloat("floatB");
    }

    if (sectionCount > 2)
    {
        atUint32 halfTransitionCount = reader.readUint32("halfTransitionCount");
        halfTransitions.reserve(halfTransitionCount);
        reader.enterSubVector("halfTransitions");
        for (int i=0 ; i<halfTransitionCount ; ++i)
        {
            halfTransitions.emplace_back();
            reader.enterSubRecord(nullptr);
            halfTransitions.back().fromYAML(reader);
            reader.leaveSubRecord();
        }
        reader.leaveSubVector();
    }

    if (sectionCount > 3)
    {
        atUint32 animResourcesCount = reader.readUint32("animResourcesCount");
        animResources.reserve(animResourcesCount);
        reader.enterSubVector("animResources");
        for (int i=0 ; i<animResourcesCount ; ++i)
        {
            animResources.emplace_back();
            reader.enterSubRecord(nullptr);
            animResources.back().fromYAML(reader);
            reader.leaveSubRecord();
        }
        reader.leaveSubVector();
    }
}

void ANCS::AnimationSet::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    atUint16 sectionCount;
    if (animResources.size())
        sectionCount = 4;
    else if (halfTransitions.size())
        sectionCount = 3;
    else if (additiveAnims.size())
        sectionCount = 2;
    else
        sectionCount = 1;

    writer.writeUint16("sectionCount", sectionCount);

    writer.writeUint32("animationCount", animations.size());
    writer.enterSubVector("animations");
    for (const auto& anim : animations)
    {
        writer.enterSubRecord(nullptr);
        anim.toYAML(writer);
        writer.leaveSubRecord();
    }
    writer.leaveSubVector();

    writer.writeUint32("transitionCount", transitions.size());
    writer.enterSubVector("transitions");
    for (const auto& trans : transitions)
    {
        writer.enterSubRecord(nullptr);
        trans.toYAML(writer);
        writer.leaveSubRecord();
    }
    writer.leaveSubVector();

    if (sectionCount > 1)
    {
        writer.writeUint32("additiveAnimCount", additiveAnims.size());
        writer.enterSubVector("additiveAnims");
        for (const auto& aa : additiveAnims)
        {
            writer.enterSubRecord(nullptr);
            aa.toYAML(writer);
            writer.leaveSubRecord();
        }
        writer.leaveSubVector();
        writer.writeFloat("floatA", floatA);
        writer.writeFloat("floatB", floatB);
    }

    if (sectionCount > 2)
    {
        writer.writeUint32("halfTransitionCount", halfTransitions.size());
        writer.enterSubVector("halfTransitions");
        for (const auto& ht : halfTransitions)
        {
            writer.enterSubRecord(nullptr);
            ht.toYAML(writer);
            writer.leaveSubRecord();
        }
        writer.leaveSubVector();
    }

    if (sectionCount > 3)
    {
        writer.writeUint32("animResourcesCount", animResources.size());
        writer.enterSubVector("animResources");
        for (const auto& ar : animResources)
        {
            writer.enterSubRecord(nullptr);
            ar.toYAML(writer);
            writer.leaveSubRecord();
        }
        writer.leaveSubVector();
    }
}

}
}

