#include <vob/aoe/physics/CollisionSystem.h>

#include <vob/aoe/physics/Material.h>
#include <vob/aoe/physics/Maths.h>

#include <glm/fwd.hpp>

#include "imgui.h"


#pragma optimize("", off)
namespace vob::aoeph
{
	void CollisionSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_fixedRateTimeContext.init(a_wdar);
		m_collisionContext.init(a_wdar);
		m_staticColliderEntities.init(a_wdar);
		m_carColliderEntities.init(a_wdar);
	}

	struct BroadPhaseCandidate
	{
		Material material;

		// The following members define a space in which triangle is inside a simple Aabb:
		// { (0, 0, 0), (x1, 0, 0), (x2, y2, 0) }
		glm::vec3 p0;
		float x1;
		float x2;
		float y2;
		glm::mat3 rotationInv;
	};

	static bool testIntersection(Aabb const& a_lhs, Aabb const& a_rhs)
	{
		return a_lhs.max.x > a_rhs.min.x && a_lhs.max.y > a_rhs.min.y && a_lhs.max.z > a_rhs.min.z
			&& a_rhs.max.x > a_lhs.min.x && a_rhs.max.y > a_lhs.min.y && a_rhs.max.z > a_lhs.min.z;
	}

	static bool testIntersection(
		glm::vec3 const& a_obbPosition,
		glm::quat const& a_obbRotationInv,
		glm::vec3 const& a_obbHalfExtentsLocal,
		glm::vec3 const& a_trianglePosition,
		glm::quat const& a_triangleRotation,
		Triangle const& a_triangleLocal)
	{
		auto const p0 = a_obbRotationInv * (a_trianglePosition + a_triangleRotation * a_triangleLocal.p0 - a_obbPosition);
		auto const p1 = a_obbRotationInv * (a_trianglePosition + a_triangleRotation * a_triangleLocal.p1 - a_obbPosition);
		auto const p2 = a_obbRotationInv * (a_trianglePosition + a_triangleRotation * a_triangleLocal.p2 - a_obbPosition);

		auto const triangleMin = glm::min(glm::min(p0, p1), p2);
		auto const triangleMax = glm::max(glm::max(p0, p1), p2);
		if (!(-a_obbHalfExtentsLocal.x < triangleMax.x && triangleMin.x < a_obbHalfExtentsLocal.x
			&& -a_obbHalfExtentsLocal.y < triangleMax.y && triangleMin.y < a_obbHalfExtentsLocal.y
			&& -a_obbHalfExtentsLocal.z < triangleMax.z && triangleMin.z < a_obbHalfExtentsLocal.z))
		{
			return false;
		}

		auto testAxis = [&](glm::vec3 const& e) -> bool
			{
				auto const xAxis = glm::vec3{ 0.0f, -e.z, e.y };
				auto const yAxis = glm::vec3{ e.z, 0.0f, -e.x };
				auto const zAxis = glm::vec3{ -e.y, e.x, 0.0f };

				for (auto const& axis : { xAxis, yAxis, zAxis })
				{
					if (lengthSquared(axis) < std::numeric_limits<float>::epsilon())
					{
						continue;
					}

					auto const r = glm::dot(a_obbHalfExtentsLocal, glm::abs(axis));
					auto const p0d = glm::dot(p0, axis);
					auto const p1d = glm::dot(p1, axis);
					auto const p2d = glm::dot(p2, axis);

					auto const pMin = std::min({ p0d, p1d, p2d });
					auto const pMax = std::max({ p0d, p1d, p2d });

					if (r < pMin || pMax < -r)
					{
						return false;
					}
				}

				return true;
			};

		auto const e0 = p1 - p0;
		auto const e1 = p2 - p1;
		auto const e2 = p0 - p2;
		if (!testAxis(e0) || !testAxis(e1) || !testAxis(e2))
		{
			return false;
		}

		auto const normal = glm::cross(e0, e1);
		assert(lengthSquared(normal) >= std::numeric_limits<float>::epsilon());
		auto const d = glm::dot(normal, p0);
		auto const r = glm::dot(a_obbHalfExtentsLocal, glm::abs(normal));
		if (r < d || d < -r)
		{
			return false;
		}

		return true;
	}

	static void collectBroadPhaseCandidates(
		glm::vec3 const& a_carPosition,
		glm::quat const& a_carRotation,
		CarCollider const& a_carCollider,
		entt::view<entt::get_t<aoest::Position const, aoest::Rotation const, StaticCollider const>> const& a_staticColliderEntities,
		std::vector<BroadPhaseCandidate>& a_broadPhaseCandidates)
	{
		a_broadPhaseCandidates.clear();

		auto const carRotationInv = glm::inverse(a_carRotation);
		auto const carBoundsCenter = a_carPosition + a_carRotation * a_carCollider.boundsCenterLocal;
		auto const carBoundsHalfExtents = glm::abs(glm::mat3_cast(a_carRotation) * a_carCollider.boundsHalfExtentsLocal);
		auto const carBounds = Aabb{ carBoundsCenter - carBoundsHalfExtents, carBoundsCenter + carBoundsHalfExtents };

		for (auto [staticEntity, staticPosition, staticRotation, staticCollider] : a_staticColliderEntities.each())
		{
			if (!testIntersection(carBounds, staticCollider.bounds))
			{
				continue;
			}

			for (auto& staticPart : staticCollider.parts)
			{
				int32_t t = 0;
				for (auto const& staticTriangleLocal : staticPart.triangles)
				{
					if (!testIntersection(
						carBoundsCenter,
						carRotationInv,
						a_carCollider.boundsHalfExtentsLocal,
						staticPosition,
						staticRotation,
						staticTriangleLocal))
					{
						continue;
					}

					auto const e0 = staticRotation * (staticTriangleLocal.p1 - staticTriangleLocal.p0);
					auto const x1 = glm::length(e0);
					auto const t0 = e0 / x1;
					auto const e2 = staticRotation * (staticTriangleLocal.p2 - staticTriangleLocal.p0);
					auto const n = glm::normalize(glm::cross(t0, e2));
					auto const t1 = glm::cross(n, t0);
					auto const rotationMatrix = glm::mat3{ t0, t1, n };
					auto const rotationMatrixInv = glm::transpose(rotationMatrix);
					auto const p2 = rotationMatrixInv * e2;
					auto const p0 = staticPosition + staticRotation * staticTriangleLocal.p0;
					a_broadPhaseCandidates.emplace_back(staticPart.material, p0, x1, p2.x, p2.y, rotationMatrixInv);
				}
			}
		}
	}

	struct Rk4CarState
	{
		struct Suspension
		{
			float length = 0.0f;
			float velocity = 0.0f;
		};

		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 linearVelocity;
		glm::vec3 angularVelocityLocal;
		std::array<Suspension, 4> suspensions;
	};

	struct Contact
	{
		Material carMaterial;
		Material staticMaterial;
		float distance = 0.0f;
		glm::vec3 carPoint = glm::vec3{ 0.0f };
		glm::vec3 staticPoint = glm::vec3{ 0.0f };
	};

	inline bool testApproximateSphereTriangleIntersection(
		glm::vec3 const& a_spherePosition,
		float const a_sphereRadius,
		glm::vec3 const& a_triangleP0,
		float const a_triangleX1,
		float const a_triangleX2,
		float const a_triangleY2,
		glm::mat3 const& a_triangleRotationInv)
	{
		auto const minX = std::min(0.0f, a_triangleX2);
		auto const maxX = std::max(a_triangleX1, a_triangleX2);
		auto const minY = 0.0f;
		auto const maxY = a_triangleY2;

		auto const pos = a_triangleRotationInv * (a_spherePosition - a_triangleP0);

		return (minX <= pos.x + a_sphereRadius && pos.x - a_sphereRadius <= maxX)
			&& (minY <= pos.y + a_sphereRadius && pos.y - a_sphereRadius <= maxY)
			&& (0 <= pos.z + a_sphereRadius && pos.z - a_sphereRadius <= 0.0f);
	}

	Contact computeClosestContact(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::vec3 const& a_radiuses,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates)
	{
		auto const rotationInv = glm::inverse(a_rotation);

		auto const maxRadius = std::max({ a_radiuses.x, a_radiuses.y, a_radiuses.z });

		Contact closestContact;
		for (auto const& [staticMaterial, p0, x1, x2, y2, triangleRotationInv] : a_broadPhaseCandidates)
		{
			if (!testApproximateSphereTriangleIntersection(a_position, maxRadius, p0, x1, x2, y2, triangleRotationInv))
			{
				continue;
			}

			auto const triangleRotation = glm::transpose(triangleRotationInv);
			auto const staticTriangle = triangle{
				p0,
				p0 + triangleRotation * glm::vec3{x1, 0.0f, 0.0f},
				p0 + triangleRotation * glm::vec3{x2, y2, 0.0f}
			};

			auto const intersectionResult = intersectEllipsoidWithTriangle(
				a_position, a_rotation, rotationInv, a_radiuses, staticTriangle);

			if (intersectionResult.signedDistance < closestContact.distance)
			{
				closestContact = Contact{
					Material{},
					staticMaterial,
					intersectionResult.signedDistance,
					intersectionResult.firstPoint,
					intersectionResult.secondPoint
				};
			}
		}

		return closestContact;
	}

	static std::vector<Contact> computeChassisPartsContacts(
		Rk4CarState const& a_carState,
		std::vector<CarCollider::ChassisPart> const& a_chassisParts,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates)
	{
		std::vector<Contact> chassisContacts;
		for (auto& chassisPart : a_chassisParts)
		{
			auto const partPosition = a_carState.position + a_carState.rotation * chassisPart.position;
			auto const partRotation = a_carState.rotation * chassisPart.rotation;

			Contact chassisContact = computeClosestContact(partPosition, partRotation, chassisPart.radiuses, a_broadPhaseCandidates);
			if (chassisContact.distance < 0.0f)
			{
				chassisContact.carMaterial = chassisPart.material;

				chassisContacts.push_back(chassisContact);
			}
		}

		return chassisContacts;
	}

	static std::array<Contact, 4> computeWheelsContacts(
		Rk4CarState const& a_carState,
		std::array<CarCollider::Wheel, 4> const& a_wheels,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates)
	{
		std::array<Contact, 4> wheelContacts;
		for (int32_t i = 0; i < 4; ++i)
		{
			auto const& wheel = a_wheels[i];
			auto const& suspensionState = a_carState.suspensions[i];

			auto const wheelPosition = a_carState.position
				+ a_carState.rotation * (wheel.suspensionAttachPosition + wheel.rotation * glm::vec3{ 0.0f, -suspensionState.length, 0.0f });
			auto const wheelRotation = a_carState.rotation * wheel.rotation;

			wheelContacts[i] = computeClosestContact(wheelPosition, wheelRotation, wheel.radiuses, a_broadPhaseCandidates);
		}

		return wheelContacts;
	}

	glm::vec3 computeContactSpringForceOver100(glm::vec3 const& a_carPoint, glm::vec3 const& a_staticPoint, float a_ellasticityOver100)
	{
		return (a_staticPoint - a_carPoint) * a_ellasticityOver100;
	}

	float computeDampingCoefficient(float a_restitution, float a_ellasticityOver100, float a_referenceMass)
	{
		auto const logRestitutionSquared = square(std::log(a_restitution));
		auto const piSquared = square(std::numbers::pi_v<float>);
		auto const zeta = std::sqrt(logRestitutionSquared / (logRestitutionSquared + piSquared));
		// zeta = glm::mix(zetaHigh, zetaLow, glm::smoothstep(0.0, 0.2, smoothstep(0.01, 0.2, 100.0 * hitSpeedNormalOver100))));
		auto const dampingCoefficient = 2.0f * 10.0f * std::sqrt(a_ellasticityOver100 * a_referenceMass) * zeta;
		return dampingCoefficient;
	}

	glm::vec3 computeContactDamperForceOver100(
		glm::vec3 const& a_carPoint, glm::vec3 const& a_staticPoint, glm::vec3 const& a_velocityOver100, float a_dampingCoefficient)
	{
		auto const hitNormal = glm::normalize(a_staticPoint - a_carPoint);
		auto const hitVelocityNormalOver100 = glm::dot(a_velocityOver100, hitNormal) * hitNormal;
		return -a_dampingCoefficient * hitVelocityNormalOver100;
	}

	glm::vec3 computeContactFrictionForceOver100(
		glm::vec3 const& a_carPoint,
		glm::vec3 const& a_staticPoint,
		glm::vec3 const& a_velocityOver100,
		glm::vec3 const& a_restitutionForceNormalOver100,
		float a_frictionFactor,
		float a_referenceMass)
	{
		auto const hitNormal = glm::normalize(a_staticPoint - a_carPoint);
		auto const hitVelocityNormalOver100 = glm::dot(a_velocityOver100, hitNormal) * hitNormal;

		glm::vec3 frictionForceOver100 = glm::vec3{ 0.0f };
		if (glm::dot(hitVelocityNormalOver100, hitVelocityNormalOver100) > square(glm::epsilon<float>()))
		{
			auto const frictionDir = -glm::normalize(hitVelocityNormalOver100);
			auto const maxFrictionOver100 = a_frictionFactor * glm::length(a_restitutionForceNormalOver100);
			frictionForceOver100 = frictionDir * std::min(maxFrictionOver100, glm::length(a_restitutionForceNormalOver100) * a_referenceMass);
		}
		return frictionForceOver100;
	}

	glm::vec3 computeContactForceOver100(
		glm::vec3 const& a_carPoint, glm::vec3 const& a_staticPoint, glm::vec3 const& a_velocityOver100, float a_mass, Material const& a_material)
	{
		auto const springForceOver100 = computeContactSpringForceOver100(a_carPoint, a_staticPoint, a_material.elasticityOver100);

		auto const dampingCoefficient = computeDampingCoefficient(a_material.restitution, a_material.elasticityOver100, a_mass);
		auto const dampenerForceOver100 = computeContactDamperForceOver100(a_carPoint, a_staticPoint, a_velocityOver100, dampingCoefficient);

		auto frictionForceOver100 = computeContactFrictionForceOver100(
			a_carPoint, a_staticPoint, a_velocityOver100, springForceOver100 + dampenerForceOver100, a_material.friction, a_mass);

		return springForceOver100 + dampenerForceOver100 + frictionForceOver100;
	}

	static inline Material combineMaterials(Material const& a_lhs, Material const& a_rhs)
	{
		return {
			1.0f / (1.0f / a_lhs.elasticityOver100 + 1.0f / a_rhs.elasticityOver100),
			std::max(a_lhs.restitution, a_rhs.restitution),
			std::sqrt(a_lhs.friction * a_rhs.friction)
		};
	}

	inline glm::quat differentiateQuaternion(glm::quat const& a_rotation, glm::vec3 const& a_angularVelocityLocal)
	{
		auto const angularVelocity = glm::quat{ 0.0f, a_angularVelocityLocal.x, a_angularVelocityLocal.y, a_angularVelocityLocal.z };
		return 0.5f * a_rotation * angularVelocity;
	}
	
	struct WheelGroundState
	{
		bool isGrounded = false;
		glm::vec3 groundPoint = glm::vec3{ 0.0f };
		glm::vec3 groundNormal = glm::vec3{ 0.0f };
		glm::vec3 tireGroundedPoint = glm::vec3{ 0.0f };
		Material groundMaterial;
	};

	void addDebugForce(
		glm::vec3 const& a_source,
		glm::vec3 const& a_force,
		aoegl::Rgba const& a_color,
		std::vector<std::tuple<glm::vec3, glm::vec3, aoegl::Rgba>>& a_debugForces)
	{
		for (auto& [source, force, color] : a_debugForces)
		{
			if (glm::length(source - a_source) < 0.1f && color == a_color)
			{
				force += a_force;
				return;
			}
		}
		a_debugForces.emplace_back(a_source, a_force, a_color);
	}

	static Rk4CarState computeDerivativeCarStateOver100(
		Rk4CarState const& a_carState,
		CarCollider const& a_carCollider,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates,
		std::array<WheelGroundState, 4>& a_wheelGroundedStates,
		std::vector<std::tuple<glm::vec3, glm::vec3, aoegl::Rgba>>& a_debugForces)
	{
		auto const rotationMatrix = glm::mat3_cast(a_carState.rotation);
		auto const rotationMatrixInv = glm::transpose(rotationMatrix);
		auto const inertia = rotationMatrix * a_carCollider.inertiaLocal * rotationMatrixInv;
		auto const inertiaInv = glm::inverse(inertia);

		auto const barycenter = a_carState.position + rotationMatrix * a_carCollider.barycenterLocal;

		auto forceOver100 = a_carCollider.force / 100.0f;
		auto torqueOver100 = a_carCollider.torque / 100.0f;

		auto const chassisPartsContacts = computeChassisPartsContacts(a_carState, a_carCollider.chassisParts, a_broadPhaseCandidates);
		for (auto const& chassisPartContact : chassisPartsContacts)
		{
			auto const material = combineMaterials(chassisPartContact.carMaterial, chassisPartContact.staticMaterial);
			auto const lever = chassisPartContact.carPoint - barycenter;
			auto const hitVelocityOver100 = a_carState.linearVelocity / 100.0f + glm::cross(rotationMatrix * a_carState.angularVelocityLocal / 100.0f, lever);

			auto const contactForceOver100 = computeContactForceOver100(
				chassisPartContact.carPoint, chassisPartContact.staticPoint, hitVelocityOver100, a_carCollider.mass / chassisPartsContacts.size(), material);

			forceOver100 += contactForceOver100;
			torqueOver100 += glm::cross(lever, contactForceOver100);
			addDebugForce(chassisPartContact.carPoint, contactForceOver100, aoegl::k_orange, a_debugForces);
		}

		auto const wheelsContacts = computeWheelsContacts(a_carState, a_carCollider.wheels, a_broadPhaseCandidates);
		std::array<float, 4> wheelForcesOver100;
		for (int32_t i = 0; i < 4; ++i)
		{
			auto const& wheel = a_carCollider.wheels[i];
			auto const& wheelContact = wheelsContacts[i];
			auto const& suspensionState = a_carState.suspensions[i];

			auto const suspensionAttachPosition = a_carState.position + a_carState.rotation * wheel.suspensionAttachPosition;
			auto const suspensionCarLever = suspensionAttachPosition - barycenter;
			auto const wheelDown = a_carState.rotation * wheel.rotation * glm::vec3{ 0.0f, -1.0f, 0.0f };

			auto suspensionForceOver100 = 0.0f;
			suspensionForceOver100 += -(suspensionState.length - wheel.suspensionRestLength) * wheel.suspensionElasticityOver100;
			suspensionForceOver100 += -suspensionState.velocity * wheel.suspensionDamperOver100;

			auto wheelForceOver100 = suspensionForceOver100;
			forceOver100 += -suspensionForceOver100 * wheelDown;
			torqueOver100 += glm::cross(suspensionCarLever, -suspensionForceOver100 * wheelDown);
			addDebugForce(suspensionAttachPosition, -suspensionForceOver100 * wheelDown, aoegl::k_red, a_debugForces);

			auto const wheelPosition = suspensionAttachPosition + wheelDown * suspensionState.length;
			if (wheelContact.distance < -std::numeric_limits<float>::epsilon())
			{
				auto const contactCarLever = wheelContact.carPoint - barycenter;
				auto const hitVelocityOver100 = a_carState.linearVelocity / 100.0f
					// + wheelDown * suspensionState.velocity / 100.0f
					+ glm::cross(rotationMatrix * a_carState.angularVelocityLocal / 100.0f, contactCarLever);

				auto const hitNormal = glm::normalize(wheelContact.staticPoint - wheelContact.carPoint);

				auto const wheelRight = a_carState.rotation * wheel.rotation * glm::vec3{ 1.0f, 0.0f, 0.0f };
				auto const contactAngleSin = glm::dot(wheelRight, hitNormal);
				auto const tireMaxAngleSin = std::cos(std::numbers::pi_v<float> / 2.0f - wheel.tireMaxAngle);

				auto const isRimContact = std::abs(contactAngleSin) > tireMaxAngleSin;
				auto const material = isRimContact
					? combineMaterials(wheelContact.carMaterial, wheelContact.staticMaterial)
					: wheel.tireMaterial;

				auto const contactForceOver100 = computeContactForceOver100(
					wheelContact.carPoint, wheelContact.staticPoint, hitVelocityOver100, wheel.mass, material);

				auto const wheelContactForceOver100 = glm::dot(contactForceOver100, wheelDown);

				if (isRimContact)
				{
					auto const chassisContactForceOver100 = contactForceOver100 - wheelContactForceOver100 * wheelDown;
					forceOver100 += chassisContactForceOver100;
					torqueOver100 += glm::cross(contactCarLever, chassisContactForceOver100);
					addDebugForce(wheelContact.carPoint, chassisContactForceOver100, aoegl::k_eggplant, a_debugForces);
				}
				else
				{
					a_wheelGroundedStates[i].isGrounded = true;
					a_wheelGroundedStates[i].groundNormal = hitNormal;
					a_wheelGroundedStates[i].groundPoint = wheelContact.staticPoint;
					a_wheelGroundedStates[i].tireGroundedPoint = wheelContact.carPoint;
					a_wheelGroundedStates[i].groundMaterial = wheelContact.staticMaterial;
				}

				wheelForceOver100 += wheelContactForceOver100;
			}

			if (suspensionState.length < 0.0f)
			{
				auto const springForceOver100 = suspensionState.length * wheel.compressedElasticityOver100 * wheelDown;
				auto const dampingCoefficient = computeDampingCoefficient(
					wheel.compressedRestitution, wheel.compressedElasticityOver100, wheel.mass);
				auto const damperForceOver100 = -dampingCoefficient * suspensionState.velocity * wheelDown / 100.0f;

				forceOver100 += springForceOver100 + damperForceOver100;
				torqueOver100 += glm::cross(suspensionCarLever, springForceOver100 + damperForceOver100);

				wheelForceOver100 -= glm::dot(springForceOver100 + damperForceOver100, wheelDown);

				addDebugForce(suspensionAttachPosition, springForceOver100 + damperForceOver100, aoegl::k_red, a_debugForces);
			}
			if (suspensionState.length > wheel.suspensionMaxLength)
			{
				// TODO: a bit meh
				wheelForceOver100 += (wheel.suspensionMaxLength - suspensionState.length) * wheel.compressedElasticityOver100;
			}

			wheelForcesOver100[i] = wheelForceOver100;
		}

		Rk4CarState derivativeCarStateOver100;
		derivativeCarStateOver100.position = a_carState.linearVelocity / 100.0f;
		derivativeCarStateOver100.linearVelocity = forceOver100 / a_carCollider.mass;
		derivativeCarStateOver100.rotation = differentiateQuaternion(a_carState.rotation, a_carState.angularVelocityLocal) / 100.0f;
		auto const angularVelocity = rotationMatrix * a_carState.angularVelocityLocal;
		derivativeCarStateOver100.angularVelocityLocal =
			rotationMatrixInv * (inertiaInv * (torqueOver100 - glm::cross(angularVelocity, inertia * angularVelocity) / 100.0f));

		for (int32_t w = 0; w < 4; ++w)
		{
			derivativeCarStateOver100.suspensions[w].length = a_carState.suspensions[w].velocity / 100.0f;
			derivativeCarStateOver100.suspensions[w].velocity = wheelForcesOver100[w] / a_carCollider.wheels[w].mass;
		}

		return derivativeCarStateOver100;
	}

	static inline glm::quat integrateQuaterion(glm::quat const& a_rotation, glm::quat const& a_rotationDerivative, float a_deltaTime)
	{
		glm::quat dqdt = a_rotationDerivative;
		glm::quat qInv = glm::conjugate(a_rotation);
		glm::quat wQuat = dqdt * qInv * 2.0f;
		glm::vec3 angularVelocityWorld = glm::vec3(wQuat.x, wQuat.y, wQuat.z);
		float angle = glm::length(angularVelocityWorld) * a_deltaTime;
		if (angle < std::numeric_limits<float>::epsilon())
		{
			return a_rotation;
		}

		glm::vec3 axis = angularVelocityWorld / glm::length(angularVelocityWorld);
		glm::quat delta = glm::angleAxis(angle, axis);
		return glm::normalize(delta * a_rotation);
	}

	static Rk4CarState integrateCarState(Rk4CarState const& a_initialCarState, Rk4CarState const& a_derivativeCarStateOver100, float a_deltaTime)
	{
		Rk4CarState newCarState = a_initialCarState;
		newCarState.position += a_derivativeCarStateOver100.position * (100.0f * a_deltaTime);
		newCarState.rotation = integrateQuaterion(newCarState.rotation, a_derivativeCarStateOver100.rotation, 100.0f * a_deltaTime);

		newCarState.linearVelocity += a_derivativeCarStateOver100.linearVelocity * (100.0f * a_deltaTime);
		// TODO: is angular velocity integration accurate?
		newCarState.angularVelocityLocal += a_derivativeCarStateOver100.angularVelocityLocal * (100.0f * a_deltaTime);

		for (int32_t i = 0; i < 4; ++i)
		{
			newCarState.suspensions[i].length += a_derivativeCarStateOver100.suspensions[i].length * (100.0f * a_deltaTime);
			newCarState.suspensions[i].velocity += a_derivativeCarStateOver100.suspensions[i].velocity * (100.0f * a_deltaTime);
		}

		return newCarState;
	}

	static void processNarrowPhaseStep(
		glm::vec3& a_carPosition,
		glm::quat& a_carRotation,
		CarCollider& a_carCollider,
		float a_duration,
		std::array<WheelGroundState, 4>& a_wheelGroundedStates,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates,
		std::vector<std::tuple<glm::vec3, glm::vec3, aoegl::Rgba>>& a_debugForces)
	{
		auto initialCarState = Rk4CarState{ a_carPosition, a_carRotation, a_carCollider.linearVelocity, a_carCollider.angularVelocityLocal };
		for (int32_t i = 0; i < 4; ++i)
		{
			initialCarState.suspensions[i] = { a_carCollider.wheels[i].suspensionLength, a_carCollider.wheels[i].suspensionVelocity };
		}

		auto const k1Over100 = computeDerivativeCarStateOver100(
			initialCarState, a_carCollider, a_broadPhaseCandidates, a_wheelGroundedStates, a_debugForces);
		auto const k2Over100 = computeDerivativeCarStateOver100(
			integrateCarState(initialCarState, k1Over100, a_duration / 2), a_carCollider, a_broadPhaseCandidates, a_wheelGroundedStates, a_debugForces);
		auto const k3Over100 = computeDerivativeCarStateOver100(
			integrateCarState(initialCarState, k2Over100, a_duration / 2), a_carCollider, a_broadPhaseCandidates, a_wheelGroundedStates, a_debugForces);
		auto const k4Over100 = computeDerivativeCarStateOver100(
			integrateCarState(initialCarState, k3Over100, a_duration), a_carCollider, a_broadPhaseCandidates, a_wheelGroundedStates, a_debugForces);

		a_carPosition += (100.0f * a_duration / 6.0f) * (k1Over100.position + 2.0f * k2Over100.position + 2.0f * k3Over100.position + k4Over100.position);
		a_carRotation += (100.0f * a_duration / 6.0f) * (k1Over100.rotation + 2.0f * k2Over100.rotation + 2.0f * k3Over100.rotation + k4Over100.rotation);
		a_carRotation = glm::normalize(a_carRotation);
		a_carCollider.linearVelocity += (100.0f * a_duration / 6.0f)
			* (k1Over100.linearVelocity + 2.0f * k2Over100.linearVelocity + 2.0f * k3Over100.linearVelocity + k4Over100.linearVelocity);
		// TODO: is angular velocity integration accurate?
		a_carCollider.angularVelocityLocal += (100.0f * a_duration / 6.0f)
			* (k1Over100.angularVelocityLocal + 2.0f * k2Over100.angularVelocityLocal + 2.0f * k3Over100.angularVelocityLocal + k4Over100.angularVelocityLocal);

		for (int32_t i = 0; i < 4; ++i)
		{
			auto const& susK1Over100 = k1Over100.suspensions[i];
			auto const& susK2Over100 = k2Over100.suspensions[i];
			auto const& susK3Over100 = k3Over100.suspensions[i];
			auto const& susK4Over100 = k4Over100.suspensions[i];

			a_carCollider.wheels[i].suspensionLength += (100.0f * a_duration / 6.0f) * (susK1Over100.length + 2.0f * susK2Over100.length + 2.0f * susK3Over100.length + susK4Over100.length);
			a_carCollider.wheels[i].suspensionVelocity += (100.0f * a_duration / 6.0f) * (susK1Over100.velocity + 2.0f * susK2Over100.velocity + 2.0f * susK3Over100.velocity + susK4Over100.velocity);
		}
	}

	static void processNarrowUpdate(
		glm::vec3& a_carPosition,
		glm::quat& a_carRotation,
		CarCollider& a_carCollider,
		float a_duration,
		int32_t a_stepCount,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates,
		std::vector<std::tuple<glm::vec3, glm::vec3, aoegl::Rgba>>& a_debugForces)
	{
		std::array<WheelGroundState, 4> wheelGroundedStates;

		auto const stepDuration = a_duration / a_stepCount;
		for (int32_t stepIndex = 0; stepIndex < a_stepCount; ++stepIndex)
		{
			processNarrowPhaseStep(a_carPosition, a_carRotation, a_carCollider, stepDuration, wheelGroundedStates, a_broadPhaseCandidates, a_debugForces);
		}

		for (int32_t i = 0; i < 4; ++i)
		{
			a_carCollider.wheels[i].isGrounded = wheelGroundedStates[i].isGrounded;
			a_carCollider.wheels[i].groundPoint = wheelGroundedStates[i].groundPoint;
			a_carCollider.wheels[i].groundNormal = wheelGroundedStates[i].groundNormal;
			a_carCollider.wheels[i].groundMaterial = wheelGroundedStates[i].groundMaterial;
		}
	}

	void CollisionSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		/*static bool k_reset = false;
		static glm::vec3 k_position = glm::vec3{ 0.0f, 2.0f, 0.0f };

		ImGui::Begin("Physics");
		ImGui::InputFloat3("Position", &k_position.x);
		if (ImGui::Button("Reset"))
		{
			k_reset = true;
		}
		ImGui::End();*/

		static std::vector<std::tuple<glm::vec3, glm::vec3, aoegl::Rgba>> k_debugForces;

		auto const& fixedRateTimeContext = m_fixedRateTimeContext.get(a_wdap);
		auto& collisionContext = m_collisionContext.get(a_wdap);
		if (collisionContext.lastFixedTickIndexProcessed == fixedRateTimeContext.tickIndex)
		{
			for (auto [source, force, color] : k_debugForces)
			{
				m_debugMeshContext.get(a_wdap).addLine(source, source + force, color);
			}
			return;
		}
		collisionContext.lastFixedTickIndexProcessed = fixedRateTimeContext.tickIndex;
		auto const tickDuration = std::chrono::duration<float>(fixedRateTimeContext.tickDuration).count();

		k_debugForces.clear();

		std::vector<BroadPhaseCandidate> broadPhaseCandidates;
		for (auto [carEntity, carPosition, carRotation, carCollider] : m_carColliderEntities.get(a_wdap).each())
		{
			/*if (k_reset)
			{
				carPosition = k_position;
				carRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
				carCollider.linearVelocity = glm::vec3{ 0.0f };
				carCollider.angularVelocityLocal = glm::vec3{ 0.0f };
				for (int32_t i = 0; i < 4; ++i)
				{
					carCollider.wheels[i].suspensionLength = 0.0f;
					carCollider.wheels[i].suspensionVelocity = 0.0f;
				}
				carCollider.force = glm::vec3{ 0.0f };
				carCollider.torque = glm::vec3{ 0.0f };
			}*/

			collectBroadPhaseCandidates(
				carPosition, carRotation, carCollider, m_staticColliderEntities.get(a_wdap), broadPhaseCandidates);

			processNarrowUpdate(
				carPosition, carRotation, carCollider, tickDuration, collisionContext.updateStepCount, broadPhaseCandidates, k_debugForces);
		}

		for (auto [source, force, color] : k_debugForces)
		{
			m_debugMeshContext.get(a_wdap).addLine(source, source + force, color);
		}
		// k_reset = false;
	}
}
