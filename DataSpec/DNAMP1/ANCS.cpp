#include "ANCS.hpp"

namespace Retro
{
namespace DNAMP1
{

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::read(Athena::io::IStreamReader& reader)
{
    parmType = reader.readUint32Big();
    unk1 = reader.readUint32Big();
    unk2 = reader.readFloatBig();
    switch (DataType(parmType))
    {
    case DataType::DTInt32:
        parmVals[0].int32 = reader.readInt32Big();
        parmVals[1].int32 = reader.readInt32Big();
        break;
    case DataType::DTUInt32:
    case DataType::DTEnum:
        parmVals[0].uint32 = reader.readUint32Big();
        parmVals[1].uint32 = reader.readUint32Big();
        break;
    case DataType::DTFloat:
        parmVals[0].float32 = reader.readFloatBig();
        parmVals[1].float32 = reader.readFloatBig();
        break;
    case DataType::DTBool:
        parmVals[0].bool1 = reader.readBool();
        parmVals[1].bool1 = reader.readBool();
        break;
    }
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::write(Athena::io::IStreamWriter& writer) const
{
    writer.writeUint32Big(parmType);
    writer.writeUint32Big(unk1);
    writer.writeFloatBig(unk2);
    switch (DataType(parmType))
    {
    case DataType::DTInt32:
        writer.writeInt32Big(parmVals[0].int32);
        writer.writeInt32Big(parmVals[1].int32);
        break;
    case DataType::DTUInt32:
    case DataType::DTEnum:
        writer.writeUint32Big(parmVals[0].uint32);
        writer.writeUint32Big(parmVals[0].uint32);
        break;
    case DataType::DTFloat:
        writer.writeFloatBig(parmVals[0].float32);
        writer.writeFloatBig(parmVals[0].float32);
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
    id = reader.readUint32Big();
    atUint32 parmInfoCount = reader.readUint32Big();
    atUint32 animInfoCount = reader.readUint32Big();

    reader.enumerate(parmInfos, parmInfoCount);

    animInfos.clear();
    animInfos.reserve(animInfoCount);
    reader.enumerate<AnimInfo>(animInfos, animInfoCount,
    [this, parmInfoCount](Athena::io::IStreamReader& reader, AnimInfo& ai)
    {
        ai.id = reader.readUint32Big();
        ai.parmVals.reserve(parmInfoCount);
        for (const ParmInfo& pi : parmInfos)
        {
            switch (ParmInfo::DataType(pi.parmType))
            {
            case ParmInfo::DTInt32:
                ai.parmVals.emplace_back(reader.readInt32Big());
                break;
            case ParmInfo::DTUInt32:
            case ParmInfo::DTEnum:
                ai.parmVals.emplace_back(reader.readUint32Big());
                break;
            case ParmInfo::DTFloat:
                ai.parmVals.emplace_back(reader.readFloatBig());
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
    writer.writeUint32Big(id);
    writer.writeUint32Big(parmInfos.size());
    writer.writeUint32Big(animInfos.size());

    for (const ParmInfo& pi : parmInfos)
        pi.write(writer);

    for (const AnimInfo& ai : animInfos)
    {
        writer.writeUint32Big(ai.id);
        auto it = ai.parmVals.begin();
        for (const ParmInfo& pi : parmInfos)
        {
            ParmInfo::Parm pVal;
            if (it != ai.parmVals.end())
                pVal = *it++;
            switch (ParmInfo::DataType(pi.parmType))
            {
            case ParmInfo::DTInt32:
                writer.writeInt32Big(pVal.int32);
                break;
            case ParmInfo::DTUInt32:
            case ParmInfo::DTEnum:
                writer.writeUint32Big(pVal.uint32);
                break;
            case ParmInfo::DTFloat:
                writer.writeFloatBig(pVal.float32);
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
    idx = reader.readUint32Big();
    atUint16 sectionCount = reader.readUint16Big();
    name = reader.readString();
    cmdl.read(reader);
    cskr.read(reader);
    cinf.read(reader);

    atUint32 animationCount = reader.readUint32Big();
    reader.enumerate(animations, animationCount);

    pasDatabase.read(reader);

    atUint32 partCount = reader.readUint32Big();
    reader.enumerate(partResData.part, partCount);

    atUint32 swhcCount = reader.readUint32Big();
    reader.enumerate(partResData.swhc, swhcCount);

    atUint32 unkCount = reader.readUint32Big();
    reader.enumerate(partResData.unk, unkCount);

    partResData.elsc.clear();
    if (sectionCount > 5)
    {
        atUint32 elscCount = reader.readUint32Big();
        reader.enumerate(partResData.elsc, elscCount);
    }

    unk1 = reader.readUint32Big();

    animAABBs.clear();
    if (sectionCount > 1)
    {
        atUint32 aabbCount = reader.readUint32Big();
        reader.enumerate(animAABBs, aabbCount);
    }

    effects.clear();
    if (sectionCount > 2)
    {
        atUint32 effectCount = reader.readUint32Big();
        reader.enumerate(effects, effectCount);
    }

    if (sectionCount > 3)
    {
        cmdlOverlay.read(reader);
        cskrOverlay.read(reader);
    }

    animIdxs.clear();
    if (sectionCount > 4)
    {
        atUint32 aidxCount = reader.readUint32Big();
        reader.enumerateBig(animIdxs, aidxCount);
    }
}

void ANCS::CharacterSet::CharacterInfo::write(Athena::io::IStreamWriter& writer) const
{
    writer.writeUint32Big(idx);

    atUint16 sectionCount;
    if (partResData.elsc.size())
        sectionCount = 6;
    else if (animIdxs.size())
        sectionCount = 5;
    else if (cmdlOverlay)
        sectionCount = 4;
    else if (effects.size())
        sectionCount = 3;
    else if (animAABBs.size())
        sectionCount = 2;
    else
        sectionCount = 1;
    writer.writeUint16Big(sectionCount);

    writer.writeString(name);
    cmdl.write(writer);
    cskr.write(writer);
    cinf.write(writer);

    writer.writeUint32Big(animations.size());
    writer.enumerate(animations);

    pasDatabase.write(writer);

    writer.writeUint32Big(partResData.part.size());
    writer.enumerate(partResData.part);

    writer.writeUint32Big(partResData.swhc.size());
    writer.enumerate(partResData.swhc);

    writer.writeUint32Big(partResData.unk.size());
    writer.enumerate(partResData.unk);

    if (sectionCount > 5)
    {
        writer.writeUint32Big(partResData.elsc.size());
        writer.enumerate(partResData.elsc);
    }

    writer.writeUint32Big(unk1);

    if (sectionCount > 1)
    {
        writer.writeUint32Big(animAABBs.size());
        writer.enumerate(animAABBs);
    }

    if (sectionCount > 2)
    {
        writer.writeUint32Big(effects.size());
        writer.enumerate(effects);
    }

    if (sectionCount > 3)
    {
        cmdlOverlay.write(writer);
        cskrOverlay.write(writer);
    }

    if (sectionCount > 4)
    {
        writer.writeUint32Big(animIdxs.size());
        for (atUint32 idx : animIdxs)
            writer.writeUint32Big(idx);
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
        reader.enumerate("cmdlOverride", cmdlOverlay);
        reader.enumerate("cskrOverride", cskrOverlay);
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
    if (partResData.elsc.size())
        sectionCount = 6;
    else if (animIdxs.size())
        sectionCount = 5;
    else if (cmdlOverlay)
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
        writer.enumerate("cmdlOverride", cmdlOverlay);
        writer.enumerate("cskrOverride", cskrOverlay);
    }

    if (sectionCount > 4)
    {
        writer.writeUint32("animIdxCount", animIdxs.size());
        writer.enumerate("animIdxs", animIdxs);
    }
}

void ANCS::AnimationSet::MetaAnimFactory::read(Athena::io::IStreamReader& reader)
{
    IMetaAnim::Type type(IMetaAnim::Type(reader.readUint32Big()));
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
    writer.writeInt32Big(m_anim->m_type);
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
    IMetaTrans::Type type(IMetaTrans::Type(reader.readUint32Big()));
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
        writer.writeInt32Big(IMetaTrans::MTNoTrans);
        return;
    }
    writer.writeInt32Big(m_trans->m_type);
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
    atUint16 sectionCount = reader.readUint16Big();

    atUint32 animationCount = reader.readUint32Big();
    reader.enumerate(animations, animationCount);

    atUint32 transitionCount = reader.readUint32Big();
    reader.enumerate(transitions, transitionCount);
    defaultTransition.read(reader);

    additiveAnims.clear();
    if (sectionCount > 1)
    {
        atUint32 additiveAnimCount = reader.readUint32Big();
        reader.enumerate(additiveAnims, additiveAnimCount);
        floatA = reader.readFloatBig();
        floatB = reader.readFloatBig();
    }

    halfTransitions.clear();
    if (sectionCount > 2)
    {
        atUint32 halfTransitionCount = reader.readUint32Big();
        reader.enumerate(halfTransitions, halfTransitionCount);
    }

    animResources.clear();
    if (sectionCount > 3)
    {
        atUint32 animResourcesCount = reader.readUint32Big();
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

    writer.writeUint16Big(sectionCount);

    writer.writeUint32Big(animations.size());
    writer.enumerate(animations);

    writer.writeUint32Big(transitions.size());
    writer.enumerate(transitions);
    defaultTransition.write(writer);

    if (sectionCount > 1)
    {
        writer.writeUint32Big(additiveAnims.size());
        writer.enumerate(additiveAnims);
        writer.writeFloatBig(floatA);
        writer.writeFloatBig(floatB);
    }

    if (sectionCount > 2)
    {
        writer.writeUint32Big(halfTransitions.size());
        writer.enumerate(halfTransitions);
    }

    if (sectionCount > 3)
    {
        writer.writeUint32Big(animResources.size());
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

