#include "CHAR.hpp"

namespace DataSpec
{
namespace DNAMP3
{

void CHAR::AnimationInfo::EVNT::SFXEvent::read(Athena::io::IStreamReader& reader)
{
    EventBase::read(reader);
    caudId.read(reader);
    unk1 = reader.readUint32Big();
    unk2 = reader.readUint32Big();
    unk3 = reader.readUint32Big();
    reader.enumerateBig(unk3Vals, unk3);
    extraType = reader.readUint32Big();
    if (extraType == 1)
        extraFloat = reader.readFloatBig();
    else if (extraType == 2)
        reader.seek(35, Athena::Current);
}

void CHAR::AnimationInfo::EVNT::SFXEvent::write(Athena::io::IStreamWriter& writer) const
{
    EventBase::write(writer);
    caudId.write(writer);
    writer.writeUint32Big(unk1);
    writer.writeUint32Big(unk2);
    writer.writeUint32Big(unk3);
    writer.enumerateBig(unk3Vals);
    writer.writeUint32Big(extraType);
    if (extraType == 1)
        writer.writeFloatBig(extraFloat);
    else if (extraType == 2)
        writer.seek(35, Athena::Current);
}

size_t CHAR::AnimationInfo::EVNT::SFXEvent::binarySize(size_t __isz) const
{
    __isz = EventBase::binarySize(__isz);
    __isz = caudId.binarySize(__isz);
    __isz += 16;
    __isz += unk3Vals.size() * 4;
    if (extraType == 1)
        __isz += 4;
    else if (extraType == 2)
        __isz += 35;
    return __isz;
}

void CHAR::AnimationInfo::EVNT::SFXEvent::read(Athena::io::YAMLDocReader& reader)
{
    EventBase::read(reader);
    reader.enumerate("caudId", caudId);
    unk1 = reader.readUint32("unk1");
    unk2 = reader.readUint32("unk2");
    unk3 = reader.readUint32("unk3");
    reader.enumerate("unk3Vals", unk3Vals, unk3);
    extraType = reader.readUint32("extraType");
    if (extraType == 1)
        extraFloat = reader.readFloat("extraFloat");
}

void CHAR::AnimationInfo::EVNT::SFXEvent::write(Athena::io::YAMLDocWriter& writer) const
{
    EventBase::write(writer);
    writer.enumerate("caudId", caudId);
    writer.writeUint32("unk1", unk1);
    writer.writeUint32("unk2", unk2);
    writer.writeUint32("unk3", unk3);
    writer.enumerate("unk3Vals", unk3Vals);
    writer.writeUint32("extraType", extraType);
    if (extraType == 1)
        writer.writeFloat("extraFloat", extraFloat);
}

const char* CHAR::AnimationInfo::EVNT::SFXEvent::DNAType()
{
    return "Retro::DNAMP3::CHAR::AnimationInfo::EVNT::SFXEvent";
}

void CHAR::AnimationInfo::MetaAnimFactory::read(Athena::io::IStreamReader& reader)
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

void CHAR::AnimationInfo::MetaAnimFactory::write(Athena::io::IStreamWriter& writer) const
{
    if (!m_anim)
        return;
    writer.writeInt32Big(atInt32(m_anim->m_type));
    m_anim->write(writer);
}

size_t CHAR::AnimationInfo::MetaAnimFactory::binarySize(size_t __isz) const
{
    if (!m_anim)
        return __isz;
    return m_anim->binarySize(__isz + 4);
}

void CHAR::AnimationInfo::MetaAnimFactory::read(Athena::io::YAMLDocReader& reader)
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

void CHAR::AnimationInfo::MetaAnimFactory::write(Athena::io::YAMLDocWriter& writer) const
{
    if (!m_anim)
        return;
    writer.writeString("type", m_anim->m_typeStr);
    m_anim->write(writer);
}

const char* CHAR::AnimationInfo::MetaAnimFactory::DNAType()
{
    return "Retro::DNAMP3::CHAR::AnimationInfo::MetaAnimFactory";
}

}
}
