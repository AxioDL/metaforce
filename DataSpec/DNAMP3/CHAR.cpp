#include "CHAR.hpp"

namespace Retro
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

void CHAR::AnimationInfo::EVNT::SFXEvent::fromYAML(Athena::io::YAMLDocReader& reader)
{
    EventBase::fromYAML(reader);
    reader.enumerate("caudId", caudId);
    unk1 = reader.readUint32("unk1");
    unk2 = reader.readUint32("unk2");
    unk3 = reader.readUint32("unk3");
    reader.enumerate("unk3Vals", unk3Vals, unk3);
    extraType = reader.readUint32("extraType");
    if (extraType == 1)
        extraFloat = reader.readFloat("extraFloat");
}

void CHAR::AnimationInfo::EVNT::SFXEvent::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    EventBase::toYAML(writer);
    writer.enumerate("caudId", caudId);
    writer.writeUint32("unk1", unk1);
    writer.writeUint32("unk2", unk2);
    writer.writeUint32("unk3", unk3);
    writer.enumerate("unk3Vals", unk3Vals);
    writer.writeUint32("extraType", extraType);
    if (extraType == 1)
        writer.writeFloat("extraFloat", extraFloat);
}

void CHAR::AnimationInfo::MetaAnimFactory::read(Athena::io::IStreamReader& reader)
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

void CHAR::AnimationInfo::MetaAnimFactory::write(Athena::io::IStreamWriter& writer) const
{
    if (!m_anim)
        return;
    writer.writeInt32Big(m_anim->m_type);
    m_anim->write(writer);
}

void CHAR::AnimationInfo::MetaAnimFactory::fromYAML(Athena::io::YAMLDocReader& reader)
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

void CHAR::AnimationInfo::MetaAnimFactory::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    if (!m_anim)
        return;
    writer.writeString("type", m_anim->m_typeStr);
    m_anim->toYAML(writer);
}

}
}
