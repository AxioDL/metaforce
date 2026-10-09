#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "rstl/algorithm.hpp"

CSteeringBehaviors::CSteeringBehaviors() : x0_(M_PIF / 2.f) {}

CVector3f CSteeringBehaviors::Flee(const CPhysicsActor& actor, const CVector3f& v0) const {
  CVector3f actVec = actor.GetTranslation() - v0;
  if (actVec.CanBeNormalized()) {
    return actVec.AsNormalized();
  }

  return actor.GetTransform().GetForward();
}

CVector3f CSteeringBehaviors::Seek(const CPhysicsActor& actor, const CVector3f& target) const {
  CVector3f posDiff = target - actor.GetTranslation();
  if (posDiff.CanBeNormalized()) {
    return posDiff.AsNormalized();
  }

  return CVector3f::Zero();
}

CVector3f CSteeringBehaviors::Arrival(const CPhysicsActor& actor, const CVector3f& dest,
                                      float dampingRadius) const {
  CVector3f posDiff = dest - actor.GetTranslation();
  if (posDiff.CanBeNormalized()) {
    if (posDiff.MagSquared() < (dampingRadius * dampingRadius)) {
      dampingRadius = posDiff.MagSquared() / (dampingRadius * dampingRadius);
    } else {
      dampingRadius = 1.f;
    }

    return dampingRadius * posDiff.AsNormalized();
  }

  return CVector3f::Zero();
}

CVector3f CSteeringBehaviors::Pursuit(const CPhysicsActor& actor, const CVector3f& v0,
                                      const CVector3f& v1) const {
  CVector3f target(CVector3f::Zero());
  const float mag = actor.GetVelocityWR().Magnitude();
  if (!ProjectLinearIntersection(actor.GetTranslation(), mag, v0, v1, target)) {
    target = v0 + v1 * 1.f;
  }

  const CVector3f ret = Seek(actor, target);
  return ret;
}

CVector3f CSteeringBehaviors::Separation(const CPhysicsActor& actor, const CVector3f& pos,
                                         float separation) const {
  CVector3f ret = CVector3f::Zero();
  const CVector3f posDiff = actor.GetTranslation() - pos;
  if (posDiff.MagSquared() < separation * separation) {
    const float t = (1.f - (posDiff.MagSquared() / (separation * separation)));
    ret =
        posDiff.CanBeNormalized() ? posDiff.AsNormalized() * t : actor.GetTransform().GetForward();
  }
  return ret;
}

CVector3f CSteeringBehaviors::Alignment(const CPhysicsActor& actor,
                                        rstl::reserved_vector< TUniqueId, 1024 >& list,
                                        const CStateManager& mgr) const {
  CVector3f align = CVector3f::Zero();

  if (!list.empty()) {
    for (int i = 0; i < list.size(); ++i) {
      if (const CActor* act = static_cast< const CActor* >(mgr.GetObjectById(list[i]))) {
        align += act->GetTransform().GetForward();
      }
    }

    align *= 1.f / list.size();
  }

  const float diff = CVector3f::GetAngleDiff(actor.GetTransform().GetForward(), align);
  align *= (diff / M_PIF);
  return align;
}

CVector3f CSteeringBehaviors::Cohesion(const CPhysicsActor& actor,
                                       rstl::reserved_vector< TUniqueId, 1024 >& list,
                                       const float dampingRadius, const CStateManager& mgr) const {
  CVector3f dest = CVector3f::Zero();
  if (!list.empty()) {
    for (int i = 0; i < list.size(); ++i) {
      if (const CActor* act = static_cast< const CActor* >(mgr.GetObjectById(list[i]))) {
        dest += act->GetTranslation();
      }
    }

    dest *= 1.f / list.size();
    return Arrival(actor, dest, dampingRadius);
  }
  return dest;
}

CVector2f CSteeringBehaviors::Flee2D(const CPhysicsActor& actor, const CVector2f& v0) const {
  const CVector2f diff = actor.GetTranslation().ToVec2f() - v0;
  if (diff.MagSquared() > FLT_EPSILON) {
    return diff.AsNormalized();
  }

  return actor.GetTransform().GetForward().ToVec2f();
}

CVector2f CSteeringBehaviors::Arrival2D(const CPhysicsActor& actor, const CVector2f& v0) const {
  const CVector2f diffVec = v0 - actor.GetTranslation().ToVec2f();
  if (diffVec.MagSquared() > FLT_EPSILON) {
    return diffVec.AsNormalized();
  }

  return CVector2f::Zero();
}

bool CSteeringBehaviors::SolveQuadratic(const float a, const float b, const float c, float& xPos,
                                        float& xNeg) {
  const float q = b * b - 4.f * a * c;
  if (q < FLT_EPSILON || fabsf(a) < FLT_EPSILON) {
    return false;
  }

  const float qSq = CMath::SqrtF(q);
  xPos = (-b + qSq) / (2.f * a);
  xNeg = (-b - qSq) / (2.f * a);
  return true;
}

bool CSteeringBehaviors::SolveCubic(const rstl::reserved_vector< float, 4 >& coefficients,
                                    rstl::reserved_vector< float, 4 >& roots) {
  if (coefficients[3] != 0.f) {
    float scale = 3.f * coefficients[3];
    float shift = coefficients[2] / scale;
    float p = coefficients[1] / scale - shift * shift;
    float pCubed = p * p * p;
    float q = -0.5f * (shift * (2.f * shift * shift) -
                       (shift * coefficients[1] - coefficients[0]) / coefficients[3]);
    float discriminant = q * q + pCubed;

    if (discriminant < 0.f) {
      float cosine = q / CMath::SqrtF(-pCubed);
      cosine = CMath::Clamp(-1.f, cosine, 1.f);
      float angle = acosf(cosine);
      float radius = 2.f * powf(-pCubed, 1.f / 6.f);
      for (float i = 0.f; i < 2.01f; i += 1.f) {
        roots.push_back(cosf((2.f * i * M_PIF + angle) / 3.f) * radius - shift);
      }

      if (roots[1] < roots[0]) {
        rstl::swap(roots[0], roots[1]);
      }
      if (roots[2] < roots[1]) {
        rstl::swap(roots[1], roots[2]);
      }
      if (roots[1] < roots[0]) {
        rstl::swap(roots[0], roots[1]);
      }
    } else {
      float sqrtDiscriminant = CMath::SqrtF(discriminant);
      float positive = powf(CMath::AbsF(q + sqrtDiscriminant), 1.f / 3.f);
      float negative = powf(CMath::AbsF(q - sqrtDiscriminant), 1.f / 3.f);
      negative = q - sqrtDiscriminant > 0.f ? negative : -negative;
      positive = q + sqrtDiscriminant > 0.f ? positive : -positive;
      roots.push_back(positive + negative - shift);
    }

    for (int i = 0; i < roots.size(); ++i) {
      float& root = roots[i];
      float derivative =
          (2.f * coefficients[2] + 3.f * root * coefficients[3]) * root + coefficients[1];
      if (derivative != 0.f) {
        root -= (((coefficients[2] + root * coefficients[3]) * root + coefficients[1]) * root +
                 coefficients[0]) /
                derivative;
      }
    }
  } else if (coefficients[2] != 0.f) {
    float shift = 0.5f * coefficients[1] / coefficients[2];
    float discriminant = shift * shift - coefficients[0] / coefficients[2];
    if (discriminant >= 0.f) {
      float sqrtDiscriminant = CMath::SqrtF(discriminant);
      roots.push_back(-shift - sqrtDiscriminant);
      roots.push_back(-shift + sqrtDiscriminant);
    }
  } else if (coefficients[1] != 0.f) {
    roots.push_back(-coefficients[0] / coefficients[1]);
  }

  return roots.size() > 0;
}

bool CSteeringBehaviors::SolveQuartic(const rstl::reserved_vector< float, 5 >& coefficients,
                                      rstl::reserved_vector< float, 4 >& roots) {
  if (coefficients[4] == 0.f) {
    rstl::reserved_vector< float, 4 > cubic;
    cubic.push_back(coefficients[0]);
    cubic.push_back(coefficients[1]);
    cubic.push_back(coefficients[2]);
    cubic.push_back(coefficients[3]);
    return SolveCubic(cubic, roots);
  } else {
    float shift = coefficients[3] / (4.f * coefficients[4]);
    float p = -6.f * shift * shift + coefficients[2] / coefficients[4];
    rstl::reserved_vector< float, 4 > cubic;
    float q = shift * (8.f * shift * shift - 2.f * coefficients[2] / coefficients[4]) +
              coefficients[1] / coefficients[4];
    float r = shift * (shift * (-3.f * shift * shift + coefficients[2] / coefficients[4]) -
                       coefficients[1] / coefficients[4]) +
              coefficients[0] / coefficients[4];

    cubic.push_back(4.f * r * p - q * q);
    cubic.push_back(-8.f * r);
    cubic.push_back(-4.f * p);
    cubic.push_back(8.f);

    rstl::reserved_vector< float, 4 > cubicRoots;
    if (SolveCubic(cubic, cubicRoots)) {
      float root = cubicRoots.back();
      float uSquared = 2.f * root - p;
      float u = CMath::SqrtF(uSquared);
      float v;
      if (u == 0.f) {
        v = root * root - r;
        if (v < 0.f) {
          return false;
        }
        v = CMath::SqrtF(v);
      } else {
        v = q / (2.f * u);
      }

      float positiveDiscriminant = uSquared - (root + v) * 4.f;
      float negativeDiscriminant = uSquared - (root - v) * 4.f;
      if (positiveDiscriminant >= 0.f) {
        float sqrtDiscriminant = CMath::SqrtF(positiveDiscriminant);
        roots.push_back((u - sqrtDiscriminant) * 0.5f - shift);
        roots.push_back((u + sqrtDiscriminant) * 0.5f - shift);
      }
      if (negativeDiscriminant >= 0.f) {
        float sqrtDiscriminant = CMath::SqrtF(negativeDiscriminant);
        roots.push_back((-u - sqrtDiscriminant) * 0.5f - shift);
        roots.push_back((-u + sqrtDiscriminant) * 0.5f - shift);
      }

      for (int i = 0; i < roots.size(); ++i) {
        float& root = roots[i];
        float derivative = ((3.f * coefficients[3] + 4.f * root * coefficients[4]) * root +
                            2.f * coefficients[2]) *
                               root +
                           coefficients[1];
        if (derivative != 0.f) {
          root -= ((((coefficients[3] + root * coefficients[4]) * root + coefficients[2]) * root +
                    coefficients[1]) *
                       root +
                   coefficients[0]) /
                  derivative;
        }
      }

      if (roots.size() > 2) {
        if (roots[2] < roots[0]) {
          rstl::swap(roots[0], roots[2]);
        }
        if (roots[3] < roots[1]) {
          rstl::swap(roots[1], roots[3]);
        }
        if (roots[1] < roots[0]) {
          rstl::swap(roots[0], roots[1]);
        }
        if (roots[3] < roots[2]) {
          rstl::swap(roots[2], roots[3]);
        }
        if (roots[2] < roots[1]) {
          rstl::swap(roots[1], roots[2]);
        }
      }
    }
    return roots.size() > 0;
  }
}

bool CSteeringBehaviors::ProjectLinearIntersection(const CVector3f& origin, float speed,
                                                   const CVector3f& position,
                                                   const CVector3f& velocity,
                                                   CVector3f& intersection) {
  CVector3f delta = position - origin;
  float positive, negative;
  if (SolveQuadratic(velocity.MagSquared() - speed * speed, CVector3f::Dot(velocity, delta) * 2.f,
                     delta.MagSquared(), positive, negative) &&
      negative > 0.f) {
    intersection = position + velocity * negative;
    return true;
  }
  return false;
}

bool CSteeringBehaviors::ProjectLinearIntersection(const CVector3f& origin, float speed,
                                                   const CVector3f& position,
                                                   const CVector3f& velocity,
                                                   const CVector3f& acceleration,
                                                   CVector3f& intersection) {
  bool found = false;
  rstl::reserved_vector< float, 5 > coefficients;
  rstl::reserved_vector< float, 4 > roots;
  CVector3f delta = position - origin;

  coefficients.push_back(delta.MagSquared());
  coefficients.push_back(CVector3f::Dot(delta, velocity) * 2.f);
  coefficients.push_back(velocity.MagSquared() + CVector3f::Dot(delta, acceleration) -
                         speed * speed);
  coefficients.push_back(CVector3f::Dot(velocity, acceleration));
  coefficients.push_back(acceleration.MagSquared() * 0.25f);

  if (SolveQuartic(coefficients, roots)) {
    for (int i = 0; i < roots.size(); ++i) {
      float time = roots[i];
      if (time > 0.f) {
        found = true;
        intersection = position + velocity * time + 0.5f * time * time * acceleration;
      }
    }
  }

  const bool result = found;
  return result;
}

bool CSteeringBehaviors::ProjectOrbitalIntersection(const CVector3f& origin, float speed, float dt,
                                                    const CVector3f& position,
                                                    const CVector3f& velocity,
                                                    const CVector3f& orbitPoint,
                                                    CVector3f& intersection) {
  if (speed > 0.f) {
    if (velocity.CanBeNormalized()) {
      CVector3f radial((position - orbitPoint).DropZ(), 0.f);
      if (radial.CanBeNormalized()) {
        CVector3f currentPosition = position;
        CVector3f currentVelocity = velocity;
        CVector3f delta = currentPosition - origin;
        float travelTime = delta.Magnitude() / speed;
        float elapsed = 0.f;
        float previousRemaining = FLT_MAX;
        float remaining = travelTime - elapsed;
        CVector3f radialUnit = radial.AsNormalized();
        CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
        float tangentialSpeed = CVector3f::Dot(currentVelocity, tangent);
        float radialSpeed = CVector3f::Dot(currentVelocity, radialUnit);

        while (remaining < previousRemaining && elapsed < 4.f) {
          if (close_enough(remaining, dt) || remaining < 0.f) {
            intersection = currentPosition;
            return true;
          }

          currentPosition += dt * currentVelocity;
          previousRemaining = remaining;
          radial = CVector3f((currentPosition - orbitPoint).DropZ(), 0.f);
          if (!radial.CanBeNormalized()) {
            break;
          }

          radialUnit = radial.AsNormalized();
          CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
          currentVelocity = tangentialSpeed * tangent + radialSpeed * radialUnit;
          delta = currentPosition - origin;
          travelTime = delta.Magnitude() / speed;
          elapsed += dt;
          remaining = travelTime - elapsed;
        }
      } else {
        return ProjectLinearIntersection(origin, speed, position, velocity, intersection);
      }
    } else {
      intersection = position;
      return true;
    }
  }
  return false;
}

bool CSteeringBehaviors::ProjectOrbitalIntersection(const CVector3f& origin, float speed, float dt,
                                                    const CVector3f& position,
                                                    const CVector3f& velocity,
                                                    const CVector3f& acceleration,
                                                    const CVector3f& orbitPoint,
                                                    CVector3f& intersection) {
  bool found = false;
  if (speed > 0.f) {
    CVector3f radial((position - orbitPoint).DropZ(), 0.f);
    if (velocity.CanBeNormalized() && radial.CanBeNormalized()) {
      CVector3f currentPosition = position;
      CVector3f currentVelocity = velocity;
      CVector3f delta = currentPosition - origin;
      float travelTime = delta.Magnitude() / speed;
      float elapsed = 0.f;
      float previousRemaining = FLT_MAX;
      float remaining = travelTime - elapsed;
      CVector3f radialUnit = radial.AsNormalized();
      CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
      float tangentialSpeed = CVector3f::Dot(currentVelocity, tangent);
      float radialSpeed = CVector3f::Dot(currentVelocity, radialUnit);

      while (remaining < previousRemaining && elapsed < 4.f) {
        if (close_enough(remaining, dt) || remaining < 0.f) {
          intersection = currentPosition;
          found = true;
          break;
        }

        currentPosition += dt * currentVelocity;
        previousRemaining = remaining;
        delta = currentPosition - origin;
        travelTime = delta.Magnitude() / speed;
        elapsed += dt;
        remaining = travelTime - elapsed;
        radial = CVector3f((currentPosition - orbitPoint).DropZ(), 0.f);
        if (!radial.CanBeNormalized()) {
          break;
        }

        radialUnit = radial.AsNormalized();
        CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
        currentVelocity = CVector3f(0.f, 0.f, currentVelocity.GetZ()) + dt * acceleration;
        currentVelocity += tangentialSpeed * tangent + radialSpeed * radialUnit;
      }
    } else {
      return ProjectLinearIntersection(origin, speed, position, velocity, acceleration,
                                       intersection);
    }
  }

  const bool result = found;
  return result;
}

CVector3f CSteeringBehaviors::ProjectOrbitalPosition(const CVector3f& position,
                                                     const CVector3f& velocity,
                                                     const CVector3f& orbitPoint, float dt,
                                                     float preThinkDt) {
  CVector3f currentPosition = position;
  if (velocity.CanBeNormalized()) {
    CVector3f radial((position - orbitPoint).DropZ(), 0.f);
    if (radial.CanBeNormalized()) {
      CVector3f currentVelocity = velocity;
      float elapsed = 0.f;
      CVector3f radialUnit = radial.AsNormalized();
      CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
      float tangentialSpeed = CVector3f::Dot(currentVelocity, tangent);
      float radialSpeed = CVector3f::Dot(currentVelocity, radialUnit);

      while (elapsed < dt) {

        currentPosition += preThinkDt * currentVelocity;
        radial = CVector3f((currentPosition - orbitPoint).DropZ(), 0.f);
        if (radial.CanBeNormalized()) {

          radialUnit = radial.AsNormalized();
          CVector3f tangent = CVector3f::Cross(radialUnit, CVector3f::Up());
          currentVelocity = tangentialSpeed * tangent + radialSpeed * radialUnit;
        }

        float step = dt - elapsed;
        if (step > preThinkDt) {
          step = preThinkDt;
        }
        elapsed += step;
      }
    }
  }
  return currentPosition;
}
