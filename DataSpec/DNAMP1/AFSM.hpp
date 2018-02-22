#ifndef _DNAMP1_AFSM_HPP_
#define _DNAMP1_AFSM_HPP_

#include <athena/FileWriter.hpp>
#include "../DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1
{
struct AFSM : public BigDNA
{
    AT_DECL_DNA_YAML
    Value<atUint32> stateCount;
    Vector<String<-1>, DNA_COUNT(stateCount)> stateNames;
    Value<atUint32> triggerCount;

    struct State : public BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint32> transitionCount;
        struct Transition : public BigDNA
        {
            AT_DECL_EXPLICIT_DNA_YAML
            Value<atUint32> triggerCount;

            struct Trigger : public BigDNA
            {
                AT_DECL_EXPLICIT_DNA_YAML
                bool first = false;
                String<-1> name;
                Value<float> parameter;
                Value<atUint32> targetState;
            };
            Vector<Trigger, DNA_COUNT(triggerCount)> triggers;
        };
        Vector<Transition, DNA_COUNT(transitionCount)> transitions;
    };
    Vector<State, DNA_COUNT(stateCount)> states;

    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
    {
        AFSM afsm;
        afsm.read(rs);
        athena::io::FileWriter writer(outPath.getAbsolutePath());
        athena::io::ToYAMLStream(afsm, writer);
        return true;
    }
};
}

#endif // _RETRO_AFSM_HPP_
