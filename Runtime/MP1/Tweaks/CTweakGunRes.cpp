#include "Runtime/MP1/Tweaks/CTweakGunRes.hpp"

namespace metaforce::MP1 {
CTweakGunRes::CTweakGunRes(CInputStream& in) {
  m_gunMotion = in.Get<std::string>();
  m_grappleArm = in.Get<std::string>();
  m_rightHand = in.Get<std::string>();
  m_powerBeam = in.Get<std::string>();
  m_iceBeam = in.Get<std::string>();
  m_waveBeam = in.Get<std::string>();
  m_plasmaBeam = in.Get<std::string>();
  m_phazonBeam = in.Get<std::string>();
  m_holoTransition = in.Get<std::string>();
  m_bombSet = in.Get<std::string>();
  m_bombExplode = in.Get<std::string>();
  m_powerBombExplode = in.Get<std::string>();
  m_powerBeamWp = in.Get<std::string>();
  m_powerBallWp = in.Get<std::string>();
  m_iceBeamWp = in.Get<std::string>();
  m_iceBallWp = in.Get<std::string>();
  m_waveBeamWp = in.Get<std::string>();
  m_waveBallWp = in.Get<std::string>();
  m_plasmaBeamWp = in.Get<std::string>();
  m_plasmaBallWp = in.Get<std::string>();
  m_phazonBeamWp = in.Get<std::string>();
  m_phazonBallWp = in.Get<std::string>();
  m_powerMuzzle = in.Get<std::string>();
  m_iceMuzzle = in.Get<std::string>();
  m_waveMuzzle = in.Get<std::string>();
  m_plasmaMuzzle = in.Get<std::string>();
  m_phazonMuzzle = in.Get<std::string>();
  m_powerCharge = in.Get<std::string>();
  m_iceCharge = in.Get<std::string>();
  m_waveCharge = in.Get<std::string>();
  m_plasmaCharge = in.Get<std::string>();
  m_phazonCharge = in.Get<std::string>();
  m_powerAuxMuzzle = in.Get<std::string>();
  m_iceAuxMuzzle = in.Get<std::string>();
  m_waveAuxMuzzle = in.Get<std::string>();
  m_plasmaAuxMuzzle = in.Get<std::string>();
  m_phazonAuxMuzzle = in.Get<std::string>();
  m_grappleSegment = in.Get<std::string>();
  m_grappleClaw = in.Get<std::string>();
  m_grappleHit = in.Get<std::string>();
  m_grappleMuzzle = in.Get<std::string>();
  m_grappleSwoosh = in.Get<std::string>();
}
} // namespace metaforce::MP1