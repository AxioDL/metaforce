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
    switch (DataType(parmType))
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
    switch (DataType(parmType))
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
    switch (DataType(parmType))
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
    switch (DataType(parmType))
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

    reader.enumerate(parmInfos, parmInfoCount);

    animInfos.clear();
    animInfos.reserve(animInfoCount);
    reader.enumerate<AnimInfo>(animInfos, animInfoCount,
    [this, parmInfoCount](Athena::io::IStreamReader& reader, AnimInfo& ai)
    {
        ai.id = reader.readUint32();
        ai.parmVals.reserve(parmInfoCount);
        for (const ParmInfo& pi : parmInfos)
        {
            switch (ParmInfo::DataType(pi.parmType))
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
    });
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
            switch (ParmInfo::DataType(pi.parmType))
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

    reader.enumerate("parmInfos", parmInfos, parmInfoCount);

    reader.enumerate<AnimInfo>("animInfos", animInfos, animInfoCount,
    [this, parmInfoCount](Athena::io::YAMLDocReader& reader, AnimInfo& ai)
    {
        ai.id = reader.readUint32("id");
        ai.parmVals.reserve(parmInfoCount);
        reader.enterSubVector("parmVals");
        for (const ParmInfo& pi : parmInfos)
        {
            switch (ParmInfo::DataType(pi.parmType))
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
    });
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    writer.writeUint32("id", id);
    writer.writeUint32("parmInfoCount", parmInfos.size());
    writer.writeUint32("animInfoCount", animInfos.size());

    writer.enumerate("parmInfos", parmInfos);

    writer.enumerate<AnimInfo>("animInfos", animInfos,
    [this](Athena::io::YAMLDocWriter& writer, const AnimInfo& ai)
    {
        writer.writeUint32("id", ai.id);
        auto it = ai.parmVals.begin();
        writer.enterSubVector("parms");
        for (const ParmInfo& pi : parmInfos)
        {
            ParmInfo::Parm pVal;
            if (it != ai.parmVals.end())
                pVal = *it++;
            switch (ParmInfo::DataType(pi.parmType))
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
    });
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
    reader.enumerate(animations, animationCount);

    pasDatabase.read(reader);

    atUint32 partCount = reader.readUint32();
    reader.enumerate(partResData.part, partCount);

    atUint32 swhcCount = reader.readUint32();
    reader.enumerate(partResData.swhc, swhcCount);

    atUint32 unkCount = reader.readUint32();
    reader.enumerate(partResData.unk, unkCount);

    partResData.elsc.clear();
    if (sectionCount > 5)
    {
        atUint32 elscCount = reader.readUint32();
        reader.enumerate(partResData.elsc, elscCount);
    }

    unk1 = reader.readUint32();
    if (sectionCount > 9)
    {
        unk2 = reader.readUint32();
        unk3 = reader.readUint32();
    }

    animAABBs.clear();
    if (sectionCount > 1)
    {
        atUint32 aabbCount = reader.readUint32();
        reader.enumerate(animAABBs, aabbCount);
    }

    effects.clear();
    if (sectionCount > 2)
    {
        atUint32 effectCount = reader.readUint32();
        reader.enumerate(effects, effectCount);
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
        reader.enumerate(animIdxs, aidxCount);
    }
}

void ANCS::CharacterSet::CharacterInfo::write(Athena::io::IStreamWriter& writer) const
{
    writer.writeUint32(idx);

    atUint16 sectionCount;
    if (unk2 || unk3)
        sectionCount = 10;
    else if (partResData.elsc.size())
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
    writer.enumerate(animations);

    pasDatabase.write(writer);

    writer.writeUint32(partResData.part.size());
    writer.enumerate(partResData.part);

    writer.writeUint32(partResData.swhc.size());
    writer.enumerate(partResData.swhc);

    writer.writeUint32(partResData.unk.size());
    writer.enumerate(partResData.unk);

    if (sectionCount > 5)
    {
        writer.writeUint32(partResData.elsc.size());
        writer.enumerate(partResData.elsc);
    }

    writer.writeUint32(unk1);
    if (sectionCount > 9)
    {
        writer.writeUint32(unk2);
        writer.writeUint32(unk3);
    }

    if (sectionCount > 1)
    {
        writer.writeUint32(animAABBs.size());
        writer.enumerate(animAABBs);
    }

    if (sectionCount > 2)
    {
        writer.writeUint32(effects.size());
        writer.enumerate(effects);
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
    reader.enumerate("cmdl", cmdl);
    reader.enumerate("cskr", cskr);
    reader.enumerate("cinf", cinf);

    atUint32 animationCount = reader.readUint32("animationCount");
    reader.enumerate("animations", animations, animationCount);

    reader.enumerate("pasDatabase", pasDatabase);

    atUint32 partCount = reader.readUint32("partCount");
    reader.enumerate("part", partResData.part, partCount);

    atUint32 swhcCount = reader.readUint32("swhcCount");
    reader.enumerate("swhc", partResData.swhc, swhcCount);

    atUint32 unkCount = reader.readUint32("unkCount");
    reader.enumerate("unk", partResData.unk, unkCount);

    partResData.elsc.clear();
    if (sectionCount > 5)
    {
        atUint32 elscCount = reader.readUint32("elscCount");
        reader.enumerate("elsc", partResData.elsc, elscCount);
    }

    unk1 = reader.readUint32("unk1");
    if (sectionCount > 9)
    {
        unk2 = reader.readUint32("unk2");
        unk3 = reader.readUint32("unk3");
    }

    animAABBs.clear();
    if (sectionCount > 1)
    {
        atUint32 aabbCount = reader.readUint32("animAABBCount");
        reader.enumerate("part", animAABBs, aabbCount);
    }

    effects.clear();
    if (sectionCount > 2)
    {
        atUint32 effectCount = reader.readUint32("effectCount");
        reader.enumerate("effects", effects, effectCount);
    }

    if (sectionCount > 3)
    {
        reader.enumerate("cmdlOverride", cmdlOverride);
        reader.enumerate("cskrOverride", cskrOverride);
    }

    animIdxs.clear();
    if (sectionCount > 4)
    {
        atUint32 animIdxCount = reader.readUint32("animIdxCount");
        reader.enumerate("animIdxs", animIdxs, animIdxCount);
    }
}

void ANCS::CharacterSet::CharacterInfo::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    writer.writeUint32("idx", idx);

    atUint16 sectionCount;
    if (unk2 || unk3)
        sectionCount = 10;
    else if (partResData.elsc.size())
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
    writer.enumerate("cmdl", cmdl);
    writer.enumerate("cskr", cskr);
    writer.enumerate("cinf", cinf);

    writer.writeUint32("animationCount", animations.size());
    writer.enumerate("animations", animations);

    writer.enumerate("pasDatabase", pasDatabase);

    writer.writeUint32("partCount", partResData.part.size());
    writer.enumerate("part", partResData.part);

    writer.writeUint32("swhcCount", partResData.swhc.size());
    writer.enumerate("swhc", partResData.swhc);

    writer.writeUint32("unkCount", partResData.unk.size());
    writer.enumerate("unk", partResData.unk);

    if (sectionCount > 5)
    {
        writer.writeUint32("elscCount", partResData.elsc.size());
        writer.enumerate("elsc", partResData.elsc);
    }

    writer.writeUint32("unk1", unk1);
    if (sectionCount > 9)
    {
        writer.writeUint32("unk2", unk2);
        writer.writeUint32("unk3", unk3);
    }

    if (sectionCount > 1)
    {
        writer.writeUint32("animAABBCount", animAABBs.size());
        writer.enumerate("animAABBs", animAABBs);
    }

    if (sectionCount > 2)
    {
        writer.writeUint32("effectCount", effects.size());
        writer.enumerate("effects", effects);
    }

    if (sectionCount > 3)
    {
        writer.enumerate("cmdlOverride", cmdlOverride);
        writer.enumerate("cskrOverride", cskrOverride);
    }

    if (sectionCount > 4)
    {
        writer.writeUint32("animIdxCount", animIdxs.size());
        writer.enumerate("animIdxs", animIdxs);
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

void ANCS::AnimationSet::MetaAnimFactory::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    if (!m_anim)
        return;
    writer.writeString("type", m_anim->m_typeStr);
    m_anim->toYAML(writer);
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

void ANCS::AnimationSet::read(Athena::io::IStreamReader& reader)
{
    atUint16 sectionCount = reader.readUint16();

    atUint32 animationCount = reader.readUint32();
    reader.enumerate(animations, animationCount);

    atUint32 transitionCount = reader.readUint32();
    reader.enumerate(transitions, transitionCount);
    defaultTransition.read(reader);

    additiveAnims.clear();
    if (sectionCount > 1)
    {
        atUint32 additiveAnimCount = reader.readUint32();
        reader.enumerate(additiveAnims, additiveAnimCount);
        floatA = reader.readFloat();
        floatB = reader.readFloat();
    }

    halfTransitions.clear();
    if (sectionCount > 2)
    {
        atUint32 halfTransitionCount = reader.readUint32();
        reader.enumerate(halfTransitions, halfTransitionCount);
    }

    animResources.clear();
    if (sectionCount > 3)
    {
        atUint32 animResourcesCount = reader.readUint32();
        reader.enumerate(animResources, animResourcesCount);
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
    writer.enumerate(animations);

    writer.writeUint32(transitions.size());
    writer.enumerate(transitions);
    defaultTransition.write(writer);

    if (sectionCount > 1)
    {
        writer.writeUint32(additiveAnims.size());
        writer.enumerate(additiveAnims);
        writer.writeFloat(floatA);
        writer.writeFloat(floatB);
    }

    if (sectionCount > 2)
    {
        writer.writeUint32(halfTransitions.size());
        writer.enumerate(halfTransitions);
    }

    if (sectionCount > 3)
    {
        writer.writeUint32(animResources.size());
        writer.enumerate(animResources);
    }
}

void ANCS::AnimationSet::fromYAML(Athena::io::YAMLDocReader& reader)
{
    atUint16 sectionCount = reader.readUint16("sectionCount");

    atUint32 animationCount = reader.readUint32("animationCount");
    reader.enumerate("animations", animations, animationCount);

    atUint32 transitionCount = reader.readUint32("transitionCount");
    reader.enumerate("transitions", transitions, transitionCount);
    reader.enumerate("defaultTransition", defaultTransition);

    additiveAnims.clear();
    if (sectionCount > 1)
    {
        atUint32 additiveAnimCount = reader.readUint32("additiveAnimCount");
        reader.enumerate("additiveAnims", additiveAnims, additiveAnimCount);
        floatA = reader.readFloat("floatA");
        floatB = reader.readFloat("floatB");
    }

    halfTransitions.clear();
    if (sectionCount > 2)
    {
        atUint32 halfTransitionCount = reader.readUint32("halfTransitionCount");
        reader.enumerate("halfTransitions", halfTransitions, halfTransitionCount);
    }

    animResources.clear();
    if (sectionCount > 3)
    {
        atUint32 animResourcesCount = reader.readUint32("animResourcesCount");
        reader.enumerate("animResources", animResources, animResourcesCount);
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
    writer.enumerate("animations", animations);

    writer.writeUint32("transitionCount", transitions.size());
    writer.enumerate("transitions", transitions);
    writer.enumerate("defaultTransition", defaultTransition);

    if (sectionCount > 1)
    {
        writer.writeUint32("additiveAnimCount", additiveAnims.size());
        writer.enumerate("additiveAnims", additiveAnims);
        writer.writeFloat("floatA", floatA);
        writer.writeFloat("floatB", floatB);
    }

    if (sectionCount > 2)
    {
        writer.writeUint32("halfTransitionCount", halfTransitions.size());
        writer.enumerate("halfTransitions", halfTransitions);
    }

    if (sectionCount > 3)
    {
        writer.writeUint32("animResourcesCount", animResources.size());
        writer.enumerate("animResources", animResources);
    }
}

}
}

