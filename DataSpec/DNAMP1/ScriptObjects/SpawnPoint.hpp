#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct SpawnPoint : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atUint32> powerBeam;
  Value<atUint32> iceBeam;
  Value<atUint32> waveBeam;
  Value<atUint32> plasmaBeam;
  Value<atUint32> missiles;
  Value<atUint32> scanVisor;
  Value<atUint32> morphBallBombs;
  Value<atUint32> powerBombs;
  Value<atUint32> flameThrower;
  Value<atUint32> thermalVisor;
  Value<atUint32> chargeBeam;
  Value<atUint32> superMissile;
  Value<atUint32> grappleBeam;
  Value<atUint32> xrayVisor;
  Value<atUint32> iceSpreader;
  Value<atUint32> spaceJump;
  Value<atUint32> morphBall;
  Value<atUint32> combatVisor;
  Value<atUint32> boostBall;
  Value<atUint32> spiderBall;
  Value<atUint32> powerSuit;
  Value<atUint32> gravitySuit;
  Value<atUint32> variaSuit;
  Value<atUint32> phazonSuit;
  Value<atUint32> energyTanks;
  Value<atUint32> unknownItem1;
  Value<atUint32> healthRefill;
  Value<atUint32> unknownItem2;
  Value<atUint32> waveBuster;
  Value<bool> defaultSpawn;
  Value<bool> active;
  Value<bool> morphed;
};
} // namespace DataSpec::DNAMP1
