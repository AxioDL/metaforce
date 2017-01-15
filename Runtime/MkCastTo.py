"Generates TCastTo source files for constexpr-enabled CEntity casts"

class Namespace:
    def __init__(self, name):
        self.name = name

class EndNamespace:
    pass

CENTITY_TYPES = (
    ('CActor', 'World/CActor.hpp'),
    ('CPhysicsActor', 'World/CPhysicsActor.hpp'),
    ('CGameCamera', 'Camera/CGameCamera.hpp'),
    ('CPatterned', 'World/CPatterned.hpp'),
    ('CGameLight', 'World/CGameLight.hpp'),
    ('CAi', 'World/CAi.hpp'),
    ('CCinematicCamera', 'Camera/CCinematicCamera.hpp'),
    ('CScriptHUDMemo', 'World/CScriptHUDMemo.hpp'),
    ('CScriptCameraHint', 'World/CScriptCameraHint.hpp'),
    ('CScriptPickup', 'World/CScriptPickup.hpp'),
    ('CScriptRandomRelay', 'World/CScriptRandomRelay.hpp'),
    ('CScriptMemoryRelay', 'World/CScriptMemoryRelay.hpp'),
    ('CScriptRelay', 'World/CScriptRelay.hpp'),
    ('CScriptCameraWaypoint', 'World/CScriptCameraWaypoint.hpp'),
    ('CScriptCoverPoint', 'World/CScriptCoverPoint.hpp'),
    ('CScriptSpawnPoint', 'World/CScriptSpawnPoint.hpp'),
    ('CRepulsor', 'World/CRepulsor.hpp'),
    ('CScriptCameraHintTrigger', 'World/CScriptCameraHintTrigger.hpp'),
    ('CScriptSwitch', 'World/CScriptSwitch.hpp'),
    ('CScriptAiJumpPoint', 'World/CScriptAiJumpPoint.hpp'),
    ('CScriptColorModulate', 'World/CScriptColorModulate.hpp'),
    ('CScriptCameraPitchVolume', 'World/CScriptCameraPitchVolume.hpp'),
    ('CPlayer', 'World/CPlayer.hpp'),
    ('CScriptActor', 'World/CScriptActor.hpp'),
    ('CScriptWaypoint', 'World/CScriptWaypoint.hpp'),
    ('CScriptDoor', 'World/CScriptDoor.hpp'),
    ('CScriptActorKeyframe', 'World/CScriptActorKeyframe.hpp'),
    ('CScriptTrigger', 'World/CScriptTrigger.hpp'),
    ('CScriptSound', 'World/CScriptSound.hpp'),
    ('CPlasmaProjectile', 'Weapon/CPlasmaProjectile.hpp'),
    ('CScriptCounter', 'World/CScriptCounter.hpp'),
    ('CScriptBeam', 'World/CScriptBeam.hpp'),
    ('CScriptStreamedMusic', 'World/CScriptStreamedMusic.hpp'),
    ('CScriptTimer', 'World/CScriptTimer.hpp'),
    ('CScriptEffect', 'World/CScriptEffect.hpp'),
    ('CScriptPlatform', 'World/CScriptPlatform.hpp'),
    ('CScriptGrapplePoint', 'World/CScriptGrapplePoint.hpp'),
    ('CScriptGenerator', 'World/CScriptGenerator.hpp'),
    ('CScriptShadowProjector', 'World/CScriptShadowProjector.hpp'),
    ('CScriptDock', 'World/CScriptDock.hpp'),
    ('CScriptAreaAttributes', 'World/CScriptAreaAttributes.hpp'),
    ('CScriptPickupGenerator', 'World/CScriptPickupGenerator.hpp'),
    ('CScriptPointOfInterest', 'World/CScriptPointOfInterest.hpp'),
    ('CScriptCameraFilterKeyframe', 'World/CScriptCameraFilterKeyframe.hpp'),
    ('CScriptCameraBlurKeyframe', 'World/CScriptCameraBlurKeyframe.hpp'),
    ('CScriptActorRotate', 'World/CScriptActorRotate.hpp'),
    ('CScriptDistanceFog', 'World/CScriptDistanceFog.hpp'),
    ('CScriptDamageableTrigger', 'World/CScriptDamageableTrigger.hpp'),
    ('CScriptDockAreaChange', 'World/CScriptDockAreaChange.hpp'),
    ('CScriptSpecialFunction', 'World/CScriptSpecialFunction.hpp'),
    ('CScriptDebris', 'World/CScriptDebris.hpp'),

    Namespace('MP1'),
    ('CBeetle', 'MP1/CBeetle.hpp', 'MP1'),
    ('CWarWasp', 'MP1/CWarWasp.hpp', 'MP1'),
    ('CSpacePirate', 'MP1/CSpacePirate.hpp', 'MP1'),
    ('CNewIntroBoss', 'MP1/CNewIntroBoss.hpp', 'MP1'),
    EndNamespace(),
)

def getqualified(tp):
    if len(tp) >= 3:
        return tp[2] + '::' + tp[0]
    else:
        return tp[0]

headerf = open('TCastTo.hpp', 'w')
sourcef = open('TCastTo.cpp', 'w')

headerf.write('''#ifndef __TCASTTO_HPP__
#define __TCASTTO_HPP__

namespace urde
{
class CEntity;
''')

for tp in CENTITY_TYPES:
    if type(tp) == tuple:
        headerf.write('class %s;\n' % tp[0])
    elif isinstance(tp, Namespace):
        headerf.write('namespace %s\n{\n' % tp.name)
    elif isinstance(tp, EndNamespace):
        headerf.write('}\n')

headerf.write('\nclass IVisitor\n{\npublic:\n')

for tp in CENTITY_TYPES:
    if type(tp) == tuple:
        headerf.write('    virtual void Visit(%s* p)=0;\n' % getqualified(tp))

headerf.write('''};

template <class T>
class TCastToPtr : public IVisitor
{
protected:
    T* ptr = nullptr;
public:
    TCastToPtr(CEntity* p);
    TCastToPtr(CEntity& p);

''')

for tp in CENTITY_TYPES:
    if type(tp) == tuple:
        headerf.write('    void Visit(%s* p);\n' % getqualified(tp))

headerf.write('''
    T* GetPtr() const { return ptr; }
    operator T*() const { return GetPtr(); }
    T* operator->() const { return GetPtr(); }
    operator bool() const { return ptr != nullptr; }
};

template <class T>
class TCastToConstPtr : TCastToPtr<T>
{
public:
    TCastToConstPtr(const CEntity* p) : TCastToPtr<T>(const_cast<CEntity*>(p)) {}
    TCastToConstPtr(const CEntity& p) : TCastToPtr<T>(const_cast<CEntity&>(p)) {}
    const T* GetPtr() const { return TCastToPtr<T>::ptr; }
    operator const T*() const { return GetPtr(); }
    const T* operator->() const { return GetPtr(); }
    operator bool() const { return TCastToPtr<T>::ptr != nullptr; }
};

}

#endif // __TCASTTO_HPP__
''')

headerf.close()

sourcef.write('#include "TCastTo.hpp"\n\n')

for tp in CENTITY_TYPES:
    if type(tp) == tuple:
        sourcef.write('#include "%s"\n' % tp[1])

sourcef.write('''
namespace urde
{

template <class T>
TCastToPtr<T>::TCastToPtr(CEntity* p) { p->Accept(*this); }

template <class T>
TCastToPtr<T>::TCastToPtr(CEntity& p) { p.Accept(*this); }

''')

for tp in CENTITY_TYPES:
    if type(tp) == tuple:
        qual = getqualified(tp)
        sourcef.write('''template <class T>
void TCastToPtr<T>::Visit(%s* p)
{
    ptr = reinterpret_cast<T*>(std::is_convertible<%s*, T*>::value ? p : nullptr);
}

''' % (qual, qual))

for tp in CENTITY_TYPES:
    if type(tp) == tuple:
        sourcef.write('template class TCastToPtr<%s>;\n' % getqualified(tp))

sourcef.write('\n}\n')

sourcef.close()
