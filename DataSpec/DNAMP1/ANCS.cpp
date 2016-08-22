#include "ANCS.hpp"

namespace DataSpec
{
extern hecl::Database::DataSpecEntry SpecEntMP1;
extern hecl::Database::DataSpecEntry SpecEntMP1PC;

namespace DNAMP1
{

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::read(athena::io::IStreamReader& reader)
{
    parmType = reader.readUint32Big();
    unk1 = reader.readUint32Big();
    unk2 = reader.readFloatBig();
    switch (DataType(parmType))
    {
    case DataType::Int32:
        parmVals[0].int32 = reader.readInt32Big();
        parmVals[1].int32 = reader.readInt32Big();
        break;
    case DataType::UInt32:
    case DataType::Enum:
        parmVals[0].uint32 = reader.readUint32Big();
        parmVals[1].uint32 = reader.readUint32Big();
        break;
    case DataType::Float:
        parmVals[0].float32 = reader.readFloatBig();
        parmVals[1].float32 = reader.readFloatBig();
        break;
    case DataType::Bool:
        parmVals[0].bool1 = reader.readBool();
        parmVals[1].bool1 = reader.readBool();
        break;
    }
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::write(athena::io::IStreamWriter& writer) const
{
    writer.writeUint32Big(parmType);
    writer.writeUint32Big(unk1);
    writer.writeFloatBig(unk2);
    switch (DataType(parmType))
    {
    case DataType::Int32:
        writer.writeInt32Big(parmVals[0].int32);
        writer.writeInt32Big(parmVals[1].int32);
        break;
    case DataType::UInt32:
    case DataType::Enum:
        writer.writeUint32Big(parmVals[0].uint32);
        writer.writeUint32Big(parmVals[0].uint32);
        break;
    case DataType::Float:
        writer.writeFloatBig(parmVals[0].float32);
        writer.writeFloatBig(parmVals[0].float32);
        break;
    case DataType::Bool:
        writer.writeBool(parmVals[0].bool1);
        writer.writeBool(parmVals[0].bool1);
        break;
    }
}

size_t ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::binarySize(size_t __isz) const
{
    __isz += 12;
    switch (DataType(parmType))
    {
    case DataType::Int32:
    case DataType::UInt32:
    case DataType::Enum:
    case DataType::Float:
        __isz += 8;
        break;
    case DataType::Bool:
        __isz += 2;
        break;
    }
    return __isz;
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::read(athena::io::YAMLDocReader& reader)
{
    parmType = reader.readUint32("parmType");
    unk1 = reader.readUint32("unk1");
    unk2 = reader.readFloat("unk2");
    size_t parmValCount;
    reader.enterSubVector("parmVals", parmValCount);
    switch (DataType(parmType))
    {
    case DataType::Int32:
        parmVals[0].int32 = reader.readInt32(nullptr);
        parmVals[1].int32 = reader.readInt32(nullptr);
        break;
    case DataType::UInt32:
    case DataType::Enum:
        parmVals[0].uint32 = reader.readUint32(nullptr);
        parmVals[1].uint32 = reader.readUint32(nullptr);
        break;
    case DataType::Float:
        parmVals[0].float32 = reader.readFloat(nullptr);
        parmVals[1].float32 = reader.readFloat(nullptr);
        break;
    case DataType::Bool:
        parmVals[0].bool1 = reader.readBool(nullptr);
        parmVals[1].bool1 = reader.readBool(nullptr);
        break;
    default: break;
    }
    reader.leaveSubVector();
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::write(athena::io::YAMLDocWriter& writer) const
{
    writer.writeUint32("parmType", parmType);
    writer.writeUint32("unk1", unk1);
    writer.writeFloat("unk2", unk2);
    writer.enterSubVector("parmVals");
    switch (DataType(parmType))
    {
    case DataType::Int32:
        writer.writeInt32(nullptr, parmVals[0].int32);
        writer.writeInt32(nullptr, parmVals[1].int32);
        break;
    case DataType::UInt32:
    case DataType::Enum:
        writer.writeUint32(nullptr, parmVals[0].uint32);
        writer.writeUint32(nullptr, parmVals[0].uint32);
        break;
    case DataType::Float:
        writer.writeFloat(nullptr, parmVals[0].float32);
        writer.writeFloat(nullptr, parmVals[0].float32);
        break;
    case DataType::Bool:
        writer.writeBool(nullptr, parmVals[0].bool1);
        writer.writeBool(nullptr, parmVals[0].bool1);
        break;
    }
    writer.leaveSubVector();
}

const char* ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::DNAType()
{
    return "urde::DNAMP1::ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo";
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::read(athena::io::IStreamReader& reader)
{
    id = reader.readUint32Big();
    atUint32 parmInfoCount = reader.readUint32Big();
    atUint32 animInfoCount = reader.readUint32Big();

    reader.enumerate(parmInfos, parmInfoCount);

    animInfos.clear();
    animInfos.reserve(animInfoCount);
    reader.enumerate<AnimInfo>(animInfos, animInfoCount,
    [this, parmInfoCount](athena::io::IStreamReader& reader, AnimInfo& ai)
    {
        ai.id = reader.readUint32Big();
        ai.parmVals.reserve(parmInfoCount);
        for (const ParmInfo& pi : parmInfos)
        {
            switch (ParmInfo::DataType(pi.parmType))
            {
            case ParmInfo::DataType::Int32:
                ai.parmVals.emplace_back(reader.readInt32Big());
                break;
            case ParmInfo::DataType::UInt32:
            case ParmInfo::DataType::Enum:
                ai.parmVals.emplace_back(reader.readUint32Big());
                break;
            case ParmInfo::DataType::Float:
                ai.parmVals.emplace_back(reader.readFloatBig());
                break;
            case ParmInfo::DataType::Bool:
                ai.parmVals.emplace_back(reader.readBool());
                break;
            default: break;
            }
        }
    });
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::write(athena::io::IStreamWriter& writer) const
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
            case ParmInfo::DataType::Int32:
                writer.writeInt32Big(pVal.int32);
                break;
            case ParmInfo::DataType::UInt32:
            case ParmInfo::DataType::Enum:
                writer.writeUint32Big(pVal.uint32);
                break;
            case ParmInfo::DataType::Float:
                writer.writeFloatBig(pVal.float32);
                break;
            case ParmInfo::DataType::Bool:
                writer.writeBool(pVal.bool1);
                break;
            default: break;
            }
        }
    }
}

size_t ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::binarySize(size_t __isz) const
{
    __isz += 12;
    __isz = __EnumerateSize(__isz, parmInfos);

    __isz += animInfos.size() * 4;
    for (const ParmInfo& pi : parmInfos)
    {
        switch (ParmInfo::DataType(pi.parmType))
        {
        case ParmInfo::DataType::Int32:
        case ParmInfo::DataType::UInt32:
        case ParmInfo::DataType::Enum:
        case ParmInfo::DataType::Float:
            __isz += animInfos.size() * 4;
            break;
        case ParmInfo::DataType::Bool:
            __isz += animInfos.size();
            break;
        default: break;
        }
    }

    return __isz;
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::read(athena::io::YAMLDocReader& reader)
{
    id = reader.readUint32("id");

    size_t parmInfoCount = reader.enumerate("parmInfos", parmInfos);

    reader.enumerate<AnimInfo>("animInfos", animInfos,
    [this, parmInfoCount](athena::io::YAMLDocReader& reader, AnimInfo& ai)
    {
        ai.id = reader.readUint32("id");
        ai.parmVals.reserve(parmInfoCount);
        size_t parmValCount;
        reader.enterSubVector("parmVals", parmValCount);
        for (const ParmInfo& pi : parmInfos)
        {
            switch (ParmInfo::DataType(pi.parmType))
            {
            case ParmInfo::DataType::Int32:
                ai.parmVals.emplace_back(reader.readInt32(nullptr));
                break;
            case ParmInfo::DataType::UInt32:
            case ParmInfo::DataType::Enum:
                ai.parmVals.emplace_back(reader.readUint32(nullptr));
                break;
            case ParmInfo::DataType::Float:
                ai.parmVals.emplace_back(reader.readFloat(nullptr));
                break;
            case ParmInfo::DataType::Bool:
                ai.parmVals.emplace_back(reader.readBool(nullptr));
                break;
            default: break;
            }
        }
        reader.leaveSubVector();
    });
}

void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::write(athena::io::YAMLDocWriter& writer) const
{
    writer.writeUint32("id", id);

    writer.enumerate("parmInfos", parmInfos);

    writer.enumerate<AnimInfo>("animInfos", animInfos,
    [this](athena::io::YAMLDocWriter& writer, const AnimInfo& ai)
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
            case ParmInfo::DataType::Int32:
                writer.writeInt32(nullptr, pVal.int32);
                break;
            case ParmInfo::DataType::UInt32:
            case ParmInfo::DataType::Enum:
                writer.writeUint32(nullptr, pVal.uint32);
                break;
            case ParmInfo::DataType::Float:
                writer.writeFloat(nullptr, pVal.float32);
                break;
            case ParmInfo::DataType::Bool:
                writer.writeBool(nullptr, pVal.bool1);
                break;
            default: break;
            }
        }
        writer.leaveSubVector();
    });
}

const char* ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::DNAType()
{
    return "urde::DNAMP1::ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState";
}

void ANCS::CharacterSet::CharacterInfo::read(athena::io::IStreamReader& reader)
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

void ANCS::CharacterSet::CharacterInfo::write(athena::io::IStreamWriter& writer) const
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
    cskr.UniqueID32::write(writer);
    cinf.UniqueID32::write(writer);

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
        cskrOverlay.UniqueID32::write(writer);
    }

    if (sectionCount > 4)
    {
        writer.writeUint32Big(animIdxs.size());
        for (atUint32 idx : animIdxs)
            writer.writeUint32Big(idx);
    }
}

size_t ANCS::CharacterSet::CharacterInfo::binarySize(size_t __isz) const
{
    __isz += 6;

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

    __isz += name.size() + 1;
    __isz += 12;

    __isz += 4;
    __isz = __EnumerateSize(__isz, animations);

    __isz = pasDatabase.binarySize(__isz);

    __isz += 4;
    __isz = __EnumerateSize(__isz, partResData.part);

    __isz += 4;
    __isz = __EnumerateSize(__isz, partResData.swhc);

    __isz += 4;
    __isz = __EnumerateSize(__isz, partResData.unk);

    if (sectionCount > 5)
    {
        __isz += 4;
        __isz = __EnumerateSize(__isz, partResData.elsc);
    }

    __isz += 4;

    if (sectionCount > 1)
    {
        __isz += 4;
        __isz = __EnumerateSize(__isz, animAABBs);
    }

    if (sectionCount > 2)
    {
        __isz += 4;
        __isz = __EnumerateSize(__isz, effects);
    }

    if (sectionCount > 3)
        __isz += 8;

    if (sectionCount > 4)
        __isz += 4 + animIdxs.size() * 4;

    return __isz;
}

void ANCS::CharacterSet::CharacterInfo::read(athena::io::YAMLDocReader& reader)
{
    idx = reader.readUint32("idx");
    atUint16 sectionCount = reader.readUint16("sectionCount");
    name = reader.readString("name");
    reader.enumerate("cmdl", cmdl);

    reader.enumerate("animations", animations);

    reader.enumerate("pasDatabase", pasDatabase);

    reader.enumerate("part", partResData.part);

    reader.enumerate("swhc", partResData.swhc);

    reader.enumerate("unk", partResData.unk);

    partResData.elsc.clear();
    if (sectionCount > 5)
    {
        reader.enumerate("elsc", partResData.elsc);
    }

    unk1 = reader.readUint32("unk1");

    animAABBs.clear();
    if (sectionCount > 1)
    {
        reader.enumerate("part", animAABBs);
    }

    effects.clear();
    if (sectionCount > 2)
    {
        reader.enumerate("effects", effects);
    }

    if (sectionCount > 3)
    {
        reader.enumerate("cmdlOverride", cmdlOverlay);
    }

    animIdxs.clear();
    if (sectionCount > 4)
    {
        reader.enumerate("animIdxs", animIdxs);
    }
}

void ANCS::CharacterSet::CharacterInfo::write(athena::io::YAMLDocWriter& writer) const
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

    writer.enumerate("animations", animations);

    writer.enumerate("pasDatabase", pasDatabase);

    writer.enumerate("part", partResData.part);

    writer.enumerate("swhc", partResData.swhc);

    writer.enumerate("unk", partResData.unk);

    if (sectionCount > 5)
    {
        writer.enumerate("elsc", partResData.elsc);
    }

    writer.writeUint32("unk1", unk1);

    if (sectionCount > 1)
    {
        writer.enumerate("animAABBs", animAABBs);
    }

    if (sectionCount > 2)
    {
        writer.enumerate("effects", effects);
    }

    if (sectionCount > 3)
    {
        writer.enumerate("cmdlOverride", cmdlOverlay);
    }

    if (sectionCount > 4)
    {
        writer.enumerate("animIdxs", animIdxs);
    }
}

const char* ANCS::CharacterSet::CharacterInfo::DNAType()
{
    return "urde::DNAMP1::ANCS::CharacterSet::CharacterInfo";
}

void ANCS::AnimationSet::MetaAnimFactory::read(athena::io::IStreamReader& reader)
{
    IMetaAnim::Type type(IMetaAnim::Type(reader.readUint32Big()));
    switch (type)
    {
    case IMetaAnim::Type::Primitive:
        m_anim.reset(new struct MetaAnimPrimitive);
        m_anim->read(reader);
        break;
    case IMetaAnim::Type::Blend:
        m_anim.reset(new struct MetaAnimBlend);
        m_anim->read(reader);
        break;
    case IMetaAnim::Type::PhaseBlend:
        m_anim.reset(new struct MetaAnimPhaseBlend);
        m_anim->read(reader);
        break;
    case IMetaAnim::Type::Random:
        m_anim.reset(new struct MetaAnimRandom);
        m_anim->read(reader);
        break;
    case IMetaAnim::Type::Sequence:
        m_anim.reset(new struct MetaAnimSequence);
        m_anim->read(reader);
        break;
    default:
        m_anim.reset(nullptr);
        break;
    }
}

void ANCS::AnimationSet::MetaAnimFactory::write(athena::io::IStreamWriter& writer) const
{
    if (!m_anim)
        return;
    writer.writeInt32Big(atUint32(m_anim->m_type));
    m_anim->write(writer);
}

size_t ANCS::AnimationSet::MetaAnimFactory::binarySize(size_t __isz) const
{
    if (!m_anim)
        return __isz;
    return m_anim->binarySize(__isz + 4);
}

void ANCS::AnimationSet::MetaAnimFactory::read(athena::io::YAMLDocReader& reader)
{
    std::string type = reader.readString("type");
    std::transform(type.begin(), type.end(), type.begin(), tolower);
    if (!type.compare("primitive"))
    {
        m_anim.reset(new struct MetaAnimPrimitive);
        m_anim->read(reader);
    }
    else if (!type.compare("blend"))
    {
        m_anim.reset(new struct MetaAnimBlend);
        m_anim->read(reader);
    }
    else if (!type.compare("phaseblend"))
    {
        m_anim.reset(new struct MetaAnimPhaseBlend);
        m_anim->read(reader);
    }
    else if (!type.compare("random"))
    {
        m_anim.reset(new struct MetaAnimRandom);
        m_anim->read(reader);
    }
    else if (!type.compare("sequence"))
    {
        m_anim.reset(new struct MetaAnimSequence);
        m_anim->read(reader);
    }
    else
    {
        m_anim.reset(nullptr);
    }

}

void ANCS::AnimationSet::MetaAnimFactory::write(athena::io::YAMLDocWriter& writer) const
{
    if (!m_anim)
        return;
    writer.writeString("type", m_anim->m_typeStr);
    m_anim->write(writer);
}

const char* ANCS::AnimationSet::MetaAnimFactory::DNAType()
{
    return "urde::DNAMP1::ANCS::AnimationSet::MetaAnimFactory";
}

void ANCS::AnimationSet::MetaTransFactory::read(athena::io::IStreamReader& reader)
{
    IMetaTrans::Type type(IMetaTrans::Type(reader.readUint32Big()));
    switch (type)
    {
    case IMetaTrans::Type::MetaAnim:
        m_trans.reset(new struct MetaTransMetaAnim);
        m_trans->read(reader);
        break;
    case IMetaTrans::Type::Trans:
        m_trans.reset(new struct MetaTransTrans);
        m_trans->read(reader);
        break;
    case IMetaTrans::Type::PhaseTrans:
        m_trans.reset(new struct MetaTransPhaseTrans);
        m_trans->read(reader);
        break;
    case IMetaTrans::Type::NoTrans:
    default:
        m_trans.reset(nullptr);
        break;
    }
}

void ANCS::AnimationSet::MetaTransFactory::write(athena::io::IStreamWriter& writer) const
{
    if (!m_trans)
    {
        writer.writeInt32Big(atUint32(IMetaTrans::Type::NoTrans));
        return;
    }
    writer.writeInt32Big(atUint32(m_trans->m_type));
    m_trans->write(writer);
}

size_t ANCS::AnimationSet::MetaTransFactory::binarySize(size_t __isz) const
{
    if (!m_trans)
        return __isz + 4;
    return m_trans->binarySize(__isz + 4);
}

void ANCS::AnimationSet::MetaTransFactory::read(athena::io::YAMLDocReader& reader)
{
    std::string type = reader.readString("type");
    std::transform(type.begin(), type.end(), type.begin(), tolower);
    if (!type.compare("metaanim"))
    {
        m_trans.reset(new struct MetaTransMetaAnim);
        m_trans->read(reader);
    }
    else if (!type.compare("trans"))
    {
        m_trans.reset(new struct MetaTransTrans);
        m_trans->read(reader);
    }
    else if (!type.compare("phasetrans"))
    {
        m_trans.reset(new struct MetaTransPhaseTrans);
        m_trans->read(reader);
    }
    else
    {
        m_trans.reset(nullptr);
    }

}

void ANCS::AnimationSet::MetaTransFactory::write(athena::io::YAMLDocWriter& writer) const
{
    if (!m_trans)
    {
        writer.writeString("type", "NoTrans");
        return;
    }
    writer.writeString("type", m_trans->m_typeStr?m_trans->m_typeStr:"NoTrans");
    m_trans->write(writer);
}

const char* ANCS::AnimationSet::MetaTransFactory::DNAType()
{
    return "urde::DNAMP1::ANCS::AnimationSet::MetaTransFactory";
}

void ANCS::AnimationSet::read(athena::io::IStreamReader& reader)
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

void ANCS::AnimationSet::write(athena::io::IStreamWriter& writer) const
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

size_t ANCS::AnimationSet::binarySize(size_t __isz) const
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

    __isz += 6;
    __isz = __EnumerateSize(__isz, animations);

    __isz += 4;
    __isz = __EnumerateSize(__isz, transitions);
    __isz = defaultTransition.binarySize(__isz);

    if (sectionCount > 1)
    {
        __isz += 4;
        __isz = __EnumerateSize(__isz, additiveAnims);
        __isz += 8;
    }

    if (sectionCount > 2)
    {
        __isz += 4;
        __isz = __EnumerateSize(__isz, halfTransitions);
    }

    if (sectionCount > 3)
    {
        __isz += 4;
        __isz = __EnumerateSize(__isz, animResources);
    }

    return __isz;
}

void ANCS::AnimationSet::read(athena::io::YAMLDocReader& reader)
{
    atUint16 sectionCount = reader.readUint16("sectionCount");

    reader.enumerate("animations", animations);

    reader.enumerate("transitions", transitions);
    reader.enumerate("defaultTransition", defaultTransition);

    additiveAnims.clear();
    if (sectionCount > 1)
    {
        reader.enumerate("additiveAnims", additiveAnims);
        floatA = reader.readFloat("floatA");
        floatB = reader.readFloat("floatB");
    }

    halfTransitions.clear();
    if (sectionCount > 2)
    {
        reader.enumerate("halfTransitions", halfTransitions);
    }

    animResources.clear();
    if (sectionCount > 3)
    {
        reader.enumerate("animResources", animResources);
    }
}

void ANCS::AnimationSet::write(athena::io::YAMLDocWriter& writer) const
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

    writer.enumerate("animations", animations);

    writer.enumerate("transitions", transitions);
    writer.enumerate("defaultTransition", defaultTransition);

    if (sectionCount > 1)
    {
        writer.enumerate("additiveAnims", additiveAnims);
        writer.writeFloat("floatA", floatA);
        writer.writeFloat("floatB", floatB);
    }

    if (sectionCount > 2)
    {
        writer.enumerate("halfTransitions", halfTransitions);
    }

    if (sectionCount > 3)
    {
        writer.enumerate("animResources", animResources);
    }
}

const char* ANCS::AnimationSet::DNAType()
{
    return "urde::DNAMP1::ANCS::AnimationSet";
}

bool ANCS::Extract(const SpecBase& dataSpec,
                   PAKEntryReadStream& rs,
                   const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter,
                   const PAK::Entry& entry,
                   bool force,
                   hecl::BlenderToken& btok,
                   std::function<void(const hecl::SystemChar*)> fileChanged)
{
    hecl::ProjectPath yamlPath = outPath.getWithExtension(_S(".yaml"));
    hecl::ProjectPath::Type yamlType = yamlPath.getPathType();
    hecl::ProjectPath blendPath = outPath.getWithExtension(_S(".blend"));
    hecl::ProjectPath::Type blendType = blendPath.getPathType();

    ANCS ancs;
    ancs.read(rs);

    if (force ||
        yamlType == hecl::ProjectPath::Type::None ||
        blendType == hecl::ProjectPath::Type::None)
    {
        if (force || yamlType == hecl::ProjectPath::Type::None)
        {
            FILE* fp = hecl::Fopen(yamlPath.getAbsolutePath().c_str(), _S("w"));
            ancs.toYAMLFile(fp);
            fclose(fp);
        }

        if (force || blendType == hecl::ProjectPath::Type::None)
        {
            hecl::BlenderConnection& conn = btok.getBlenderConnection();
            DNAANCS::ReadANCSToBlender<PAKRouter<PAKBridge>, ANCS, MaterialSet, DNACMDL::SurfaceHeader_1, 2>
                    (conn, ancs, blendPath, pakRouter, entry, dataSpec, fileChanged, force);
        }
    }

    /* Extract EVNTs */
    std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>> animRes;
    ancs.getAnimationResInfo(animRes);
    for (const auto& res : animRes)
    {
        if (res.second.evntId)
        {
            hecl::SystemStringView sysStr(res.second.name);
            hecl::ProjectPath evntYamlPath = outPath.getWithExtension((hecl::SystemString(_S(".")) +
                                                                       sysStr.sys_str() +
                                                                       _S(".evnt.yaml")).c_str());
            hecl::ProjectPath::Type evntYamlType = evntYamlPath.getPathType();

            if (force || evntYamlType == hecl::ProjectPath::Type::None)
            {
                EVNT evnt;
                if (pakRouter.lookupAndReadDNA(res.second.evntId, evnt, true))
                {
                    FILE* fp = hecl::Fopen(evntYamlPath.getAbsolutePath().c_str(), _S("w"));
                    evnt.toYAMLFile(fp);
                    fclose(fp);
                }
            }
        }
    }

    return true;
}

bool ANCS::Cook(const hecl::ProjectPath& outPath,
                const hecl::ProjectPath& inPath,
                const DNAANCS::Actor& actor,
                hecl::BlenderConnection::DataStream& ds,
                const std::function<bool(const hecl::ProjectPath& modelPath)>& modelCookFunc)
{
    /* Search for yaml */
    hecl::ProjectPath yamlPath = inPath.getWithExtension(_S(".yaml"), true);
    if (yamlPath.getPathType() != hecl::ProjectPath::Type::File)
        Log.report(logvisor::Fatal, _S("'%s' not found as file"),
                   yamlPath.getRelativePath().c_str());

    FILE* yamlFp = hecl::Fopen(yamlPath.getAbsolutePath().c_str(), _S("r"));
    if (!yamlFp)
        Log.report(logvisor::Fatal, _S("can't open '%s' for reading"),
                   yamlPath.getRelativePath().c_str());

    if (!BigYAML::ValidateFromYAMLFile<ANCS>(yamlFp))
    {
        Log.report(logvisor::Fatal, _S("'%s' is not urde::DNAMP1::ANCS type"),
                   yamlPath.getRelativePath().c_str());
    }

    athena::io::YAMLDocReader yamlReader;
    yaml_parser_set_input_file(yamlReader.getParser(), yamlFp);
    if (!yamlReader.parse())
    {
        Log.report(logvisor::Fatal, _S("unable to parse '%s'"),
                   yamlPath.getRelativePath().c_str());
    }
    fclose(yamlFp);
    ANCS ancs;
    ancs.read(yamlReader);

    /* Set Character Resource IDs */
    for (ANCS::CharacterSet::CharacterInfo& ch : ancs.characterSet.characters)
    {
        hecl::SystemStringView chSysName(ch.name);
        ch.cskr = inPath.getWithExtension((_S('.') + chSysName.sys_str()).c_str(), true).ensureAuxInfo(_S("CSKR"));

        for (const DNAANCS::Actor::Subtype& sub : actor.subtypes)
        {
            if (!sub.name.compare(ch.name))
            {
                if (sub.armature >= 0)
                {
                    const DNAANCS::Actor::Armature& arm = actor.armatures[sub.armature];
                    hecl::SystemStringView armSysName(arm.name);
                    ch.cinf = inPath.getWithExtension((_S('.') + armSysName.sys_str()).c_str(), true).ensureAuxInfo(_S("CINF"));
                    break;
                }
            }
        }

        ch.cskrOverlay = inPath.getWithExtension((_S('.') + chSysName.sys_str() + _S(".over")).c_str(), true).ensureAuxInfo(_S("CSKR"));
    }

    /* Set Animation Resource IDs */
    ancs.enumeratePrimitives([&](AnimationSet::MetaAnimPrimitive& prim) -> bool
    {
        hecl::SystemStringView sysStr(prim.animName);
        hecl::ProjectPath pathOut = inPath.getWithExtension((_S('.') + sysStr.sys_str()).c_str(), true).ensureAuxInfo(_S("ANIM"));
        prim.animId = pathOut;
        return true;
    });

    std::unordered_map<std::string, atInt32> boneIdMap;
    std::experimental::optional<DNAANIM::RigInverter<CINF>> rigInv;

    /* Write out CINF resources */
    for (const DNAANCS::Actor::Armature& arm : actor.armatures)
    {        
        hecl::SystemStringView sysStr(arm.name);
        hecl::ProjectPath pathOut = inPath.getCookedPath(SpecEntMP1).getWithExtension((_S('.') +
                                    sysStr.sys_str()).c_str(), true).ensureAuxInfo(_S("CINF"));
        athena::io::FileWriter w(pathOut.getAbsolutePath(), true, false);
        if (w.hasError())
            Log.report(logvisor::Fatal, _S("unable to open '%s' for writing"),
                       pathOut.getRelativePath().c_str());
        CINF cinf(arm, boneIdMap);
        cinf.write(w);

        if (!rigInv)
        {
            auto matrices = ds.getBoneMatrices(arm.name);
            rigInv.emplace(cinf, matrices);
        }
    }
    ds.close();

    /* Write out CSKR resources */
    for (ANCS::CharacterSet::CharacterInfo& ch : ancs.characterSet.characters)
    {
        const DNAANCS::Actor::Subtype* subtype = nullptr;
        for (const DNAANCS::Actor::Subtype& sub : actor.subtypes)
        {
            if (!sub.name.compare(ch.name))
            {
                subtype = &sub;
                break;
            }
        }
        if (!subtype)
            Log.report(logvisor::Fatal, "unable to find subtype '%s'", ch.name.c_str());

        const hecl::ProjectPath& modelPath = subtype->mesh;

        if (modelPath.getPathType() != hecl::ProjectPath::Type::File)
            Log.report(logvisor::Fatal, _S("unable to resolve '%s'"), modelPath.getRelativePath().c_str());

        hecl::ProjectPath skinIntPath = modelPath.getCookedPath(SpecEntMP1PC).getWithExtension(_S(".skinint"));
        if (skinIntPath.getPathType() != hecl::ProjectPath::Type::File ||
            skinIntPath.getModtime() < modelPath.getModtime())
            if (!modelCookFunc(modelPath))
                Log.report(logvisor::Fatal, _S("unable to cook '%s'"), modelPath.getRelativePath().c_str());

        athena::io::FileReader skinIO(skinIntPath.getAbsolutePath(), 1024*32, false);
        if (skinIO.hasError())
            Log.report(logvisor::Fatal, _S("unable to open '%s'"), skinIntPath.getRelativePath().c_str());

        std::vector<std::vector<uint32_t>> skinBanks;
        uint32_t bankCount = skinIO.readUint32Big();
        skinBanks.reserve(bankCount);
        for (uint32_t i=0 ; i<bankCount ; ++i)
        {
            skinBanks.emplace_back();
            std::vector<uint32_t>& bonesOut = skinBanks.back();
            uint32_t boneCount = skinIO.readUint32Big();
            bonesOut.reserve(boneCount);
            for (uint32_t j=0 ; j<boneCount ; ++j)
            {
                uint32_t idx = skinIO.readUint32Big();
                bonesOut.push_back(idx);
            }
        }

        std::vector<std::string> boneNames;
        uint32_t boneNameCount = skinIO.readUint32Big();
        boneNames.reserve(boneNameCount);
        for (uint32_t i=0 ; i<boneNameCount ; ++i)
            boneNames.push_back(skinIO.readString());

        skinIO.close();

        hecl::SystemStringView sysStr(ch.name);
        hecl::ProjectPath skinPath = inPath.getCookedPath(SpecEntMP1PC).getWithExtension((_S('.') +
                                     sysStr.sys_str()).c_str(), true).ensureAuxInfo(_S("CSKR"));
        athena::io::FileWriter skinOut(skinPath.getAbsolutePath(), true, false);
        if (skinOut.hasError())
            Log.report(logvisor::Fatal, _S("unable to open '%s' for writing"),
                       skinPath.getRelativePath().c_str());

        skinOut.writeUint32Big(bankCount);
        for (const std::vector<uint32_t>& bank : skinBanks)
        {
            skinOut.writeUint32Big(bank.size());
            for (uint32_t bIdx : bank)
            {
                const std::string& name = boneNames[bIdx];
                auto search = boneIdMap.find(name);
                if (search == boneIdMap.cend())
                    Log.report(logvisor::Fatal, "unable to find bone '%s' in %s",
                               name.c_str(), inPath.getRelativePathUTF8().c_str());
                skinOut.writeUint32Big(search->second);
            }
        }
    }

    /* Write out ANIM resources */
    ancs.animationSet.animResources.reserve(actor.actions.size());
    for (const DNAANCS::Actor::Action& act : actor.actions)
    {
        hecl::SystemStringView sysStr(act.name);
        hecl::ProjectPath pathOut = inPath.getWithExtension((_S('.') + sysStr.sys_str()).c_str(), true).ensureAuxInfo(_S("ANIM"));
        hecl::ProjectPath cookedOut = inPath.getCookedPath(SpecEntMP1).getWithExtension((_S('.') + sysStr.sys_str()).c_str(), true);
        athena::io::FileWriter w(cookedOut.getAbsolutePath(), true, false);
        if (w.hasError())
            Log.report(logvisor::Fatal, _S("unable to open '%s' for writing"),
                       cookedOut.getRelativePath().c_str());
        ANIM anim(act, boneIdMap, *rigInv);

        ancs.animationSet.animResources.emplace_back();
        ancs.animationSet.animResources.back().animId = pathOut;

        /* Check for associated EVNT YAML */
        hecl::ProjectPath evntYamlPath = inPath.getWithExtension((hecl::SystemString(_S(".")) +
                                                                  sysStr.sys_str() +
                                                                  _S(".evnt.yaml")).c_str(), true);
        if (evntYamlPath.getPathType() == hecl::ProjectPath::Type::File)
        {
            FILE* fp = hecl::Fopen(evntYamlPath.getAbsolutePath().c_str(), _S("r"));
            if (fp)
            {
                EVNT evnt;
                evnt.fromYAMLFile(fp);
                fclose(fp);
                anim.m_anim->evnt = evntYamlPath;

                hecl::ProjectPath evntYamlOut = cookedOut.getWithExtension(_S(".evnt"));
                athena::io::FileWriter w(evntYamlOut.getAbsolutePath(), true, false);
                if (w.hasError())
                    Log.report(logvisor::Fatal, _S("unable to open '%s' for writing"),
                               evntYamlOut.getRelativePath().c_str());

                evnt.write(w);
                ancs.animationSet.animResources.back().evntId = evntYamlPath;
            }
        }

        anim.write(w);
    }

    /* Write out ANCS */
    athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
    if (w.hasError())
        Log.report(logvisor::Fatal, _S("unable to open '%s' for writing"),
                   outPath.getRelativePath().c_str());
    ancs.write(w);

    return true;
}

}
}

