#include <vob/aoe/physics/CollisionSystem.h>

#include <vob/aoe/physics/Material.h>
#include <vob/aoe/physics/MathUtils.h>

#include <glm/fwd.hpp>

#include <vob/misc/physics/measure_literals.h>
#include <vob/misc/std/ignorable_assert.h>

#include "imgui.h"


namespace vob::aoeph
{
	static bool k_disableIgnorableAsserts = true;
	static bool k_immediateStop = false;

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

#pragma optimize("", off)
	static void collectBroadPhaseCandidates(
		glm::vec3 const& a_carPosition,
		glm::quat const& a_carRotation,
		CarCollider& a_carCollider,
		entt::view<entt::get_t<aoest::Position const, aoest::Rotation const, StaticCollider const>> const& a_staticColliderEntities,
		std::vector<BroadPhaseCandidate>& a_broadPhaseCandidates)
	{
		a_broadPhaseCandidates.clear();
		a_carCollider.broadPhaseCandidates.clear();

		auto const carBoundsCenter = a_carPosition + a_carRotation * a_carCollider.boundsCenterLocal;
		auto const carBounds = computeBounds(carBoundsCenter, a_carRotation, a_carCollider.boundsHalfExtentsLocal);
		
		auto const carRotationInv = glm::inverse(a_carRotation);
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
					ignorable_assert(k_disableIgnorableAsserts || glm::length(glm::cross(t0, e2)) > 0.000001f);
					auto const n = glm::normalize(glm::cross(t0, e2));
					auto const t1 = glm::cross(n, t0);
					auto const rotationMatrix = glm::mat3{ t0, t1, n };
					auto const rotationMatrixInv = glm::transpose(rotationMatrix);
					auto const p2 = rotationMatrixInv * e2;
					auto const p0 = staticPosition + staticRotation * staticTriangleLocal.p0;
					a_broadPhaseCandidates.emplace_back(staticPart.material, p0, x1, p2.x, p2.y, rotationMatrixInv);
					a_carCollider.broadPhaseCandidates.emplace_back(
						staticPosition + staticRotation * staticTriangleLocal.p0,
						staticPosition + staticRotation * staticTriangleLocal.p1,
						staticPosition + staticRotation * staticTriangleLocal.p2);
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
		float distance = 0.0f;
		glm::vec3 carPoint = glm::vec3{ 0.0f };
		glm::vec3 staticPoint = glm::vec3{ 0.0f };
		int32_t index = 0; // index of part
		Material carMaterial;
		Material staticMaterial;
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
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates,
		std::vector<int32_t>& a_narrowPhaseContacts)
	{
		auto const rotationInv = glm::inverse(a_rotation);

		auto const maxRadius = std::max({ a_radiuses.x, a_radiuses.y, a_radiuses.z });

		Contact closestContact;
		auto candidateIndex = 0;
		for (auto const& [staticMaterial, p0, x1, x2, y2, triangleRotationInv] : a_broadPhaseCandidates)
		{
			if (!testApproximateSphereTriangleIntersection(a_position, maxRadius, p0, x1, x2, y2, triangleRotationInv))
			{
				++candidateIndex;
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
				if (std::find(a_narrowPhaseContacts.begin(), a_narrowPhaseContacts.end(), candidateIndex) == a_narrowPhaseContacts.end())
				{
					a_narrowPhaseContacts.emplace_back(candidateIndex);
				}

				closestContact = Contact{
					intersectionResult.signedDistance,
					intersectionResult.firstPoint,
					intersectionResult.secondPoint,
					-1 /* index */,
					Material{},
					staticMaterial
				};
			}

			++candidateIndex;
		}

		return closestContact;
	}

	static std::vector<Contact> computeChassisPartsContacts(
		Rk4CarState const& a_carState,
		std::vector<CarCollider::ChassisPart> const& a_chassisParts,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates,
		std::vector<int32_t>& a_narrowPhaseContacts)
	{
		std::vector<Contact> chassisContacts;
		int32_t index = 0;
		for (auto& chassisPart : a_chassisParts)
		{
			auto const partPosition = a_carState.position + a_carState.rotation * chassisPart.position;
			auto const partRotation = a_carState.rotation * chassisPart.rotation;

			Contact chassisContact = computeClosestContact(
				partPosition, partRotation, chassisPart.radiuses, a_broadPhaseCandidates, a_narrowPhaseContacts);
			if (chassisContact.distance < 0.0f)
			{
				chassisContact.carMaterial = chassisPart.material;
				chassisContact.index = index;
				chassisContacts.push_back(chassisContact);
			}

			++index;
		}

		return chassisContacts;
	}

	static std::array<Contact, 4> computeWheelsContacts(
		Rk4CarState const& a_carState,
		std::array<CarCollider::Wheel, 4> const& a_wheels,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates,
		std::vector<int32_t>& a_narrowPhaseContacts)
	{
		std::array<Contact, 4> wheelContacts;
		for (int32_t i = 0; i < 4; ++i)
		{
			auto const& wheel = a_wheels[i];
			auto const& suspensionState = a_carState.suspensions[i];

			auto const wheelPosition = a_carState.position
				+ a_carState.rotation * (wheel.suspensionAttachPosition + wheel.rotation * glm::vec3{ 0.0f, -suspensionState.length, 0.0f });
			auto const wheelRotation = a_carState.rotation * wheel.rotation;

			wheelContacts[i] = computeClosestContact(
				wheelPosition, wheelRotation, wheel.radiuses, a_broadPhaseCandidates, a_narrowPhaseContacts);
			wheelContacts[i].index = i;
		}

		return wheelContacts;
	}

	glm::vec3 computeContactSpringForceOver100(glm::vec3 const& a_carPoint, glm::vec3 const& a_staticPoint, float a_elasticityOver100)
	{
		// TODO: do I want to clamp it really?
		static float k_maxPenetrationDist = 0.25f;
		auto const penetration = (a_staticPoint - a_carPoint);
		auto const penetrationDist2 = glm::length2(penetration);
		auto const clampedPenetration = penetrationDist2 > k_maxPenetrationDist * k_maxPenetrationDist ? penetration * (k_maxPenetrationDist / penetrationDist2) : penetration;
		return clampedPenetration * a_elasticityOver100;
	}

	float computeDampingCoefficient(float a_restitution, float a_elasticityOver100, float a_referenceMass)
	{
		auto const logRestitutionSquared = square(std::log(a_restitution));
		auto const piSquared = square(std::numbers::pi_v<float>);
		auto const zeta = std::sqrt(logRestitutionSquared / (logRestitutionSquared + piSquared));
		// zeta = glm::mix(zetaHigh, zetaLow, glm::smoothstep(0.0, 0.2, smoothstep(0.01, 0.2, 100.0 * hitSpeedNormalOver100))));
		auto const dampingCoefficient = 2.0f * 10.0f * std::sqrt(a_elasticityOver100 * a_referenceMass) * zeta;
		return dampingCoefficient;
	}

	glm::vec3 computeContactDamperForceOver100(
		glm::vec3 const& a_carPoint, glm::vec3 const& a_staticPoint, glm::vec3 const& a_velocityOver100, float a_dampingCoefficient)
	{
		ignorable_assert(k_disableIgnorableAsserts || glm::length(a_staticPoint - a_carPoint) > 0.000001f);
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
		ignorable_assert(k_disableIgnorableAsserts || glm::length(a_staticPoint - a_carPoint) > 0.000001f);
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
		CarCollider /*const*/& a_carCollider,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates,
		std::array<WheelGroundState, 4>& a_wheelGroundedStates,
		int32_t a_rk4Step)
	{
		auto const rotationMatrix = glm::mat3_cast(a_carState.rotation);
		auto const rotationMatrixInv = glm::transpose(rotationMatrix);
		auto const inertia = rotationMatrix * a_carCollider.inertiaLocal * rotationMatrixInv;
		auto const inertiaInv = glm::inverse(inertia);

		auto const barycenter = a_carState.position + rotationMatrix * a_carCollider.barycenterLocal;

		auto forceOver100 = a_carCollider.force / 100.0f;
		auto torqueOver100 = a_carCollider.torque / 100.0f;
		ignorable_assert(k_disableIgnorableAsserts || glm::length(torqueOver100) < 1'000'000.0f);

		auto const chassisPartsContacts = computeChassisPartsContacts(
			a_carState, a_carCollider.chassisParts, a_broadPhaseCandidates, a_carCollider.narrowPhaseContacts);
		for (auto const& chassisPartContact : chassisPartsContacts)
		{
			auto const material = combineMaterials(chassisPartContact.carMaterial, chassisPartContact.staticMaterial);
			auto const lever = chassisPartContact.carPoint - barycenter;
			auto const hitVelocityOver100 = a_carState.linearVelocity / 100.0f + glm::cross(rotationMatrix * a_carState.angularVelocityLocal / 100.0f, lever);

			auto const contactForceOver100 = computeContactForceOver100(
				chassisPartContact.carPoint, chassisPartContact.staticPoint, hitVelocityOver100, a_carCollider.mass / chassisPartsContacts.size(), material);

			forceOver100 += contactForceOver100;
			torqueOver100 += glm::cross(lever, contactForceOver100);
			ignorable_assert(k_disableIgnorableAsserts || glm::length(torqueOver100) < 1'000'000.0f);

			// DEBUG
			a_carCollider.chassisParts[chassisPartContact.index].contacts.back()[a_rk4Step].push_back(DebugContact{
				.carPoint = chassisPartContact.carPoint,
				.staticPoint = chassisPartContact.staticPoint,
				.force = contactForceOver100,
				.torque = glm::cross(lever, contactForceOver100)
			});
		}

		auto const wheelsContacts = computeWheelsContacts(
			a_carState, a_carCollider.wheels, a_broadPhaseCandidates, a_carCollider.narrowPhaseContacts);
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
			ignorable_assert(k_disableIgnorableAsserts || glm::length(torqueOver100) < 1'000'000.0f);

			auto const wheelPosition = suspensionAttachPosition + wheelDown * suspensionState.length;
			if (wheelContact.distance < -std::numeric_limits<float>::epsilon())
			{
				auto const contactCarLever = wheelContact.carPoint - barycenter;
				auto const hitVelocityOver100 = a_carState.linearVelocity / 100.0f
					// + wheelDown * suspensionState.velocity / 100.0f
					+ glm::cross(rotationMatrix * a_carState.angularVelocityLocal / 100.0f, contactCarLever);

				ignorable_assert(k_disableIgnorableAsserts || glm::length(wheelContact.staticPoint - wheelContact.carPoint) > 0.000001f);
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
					ignorable_assert(k_disableIgnorableAsserts || glm::length(torqueOver100) < 1'000'000.0f);

					// DEBUG
					a_carCollider.wheels[wheelContact.index].chassisContacts.back()[a_rk4Step].push_back(DebugContact{
						.carPoint = wheelContact.carPoint,
						.staticPoint = wheelContact.staticPoint,
						.force = chassisContactForceOver100,
						.torque = glm::cross(contactCarLever, chassisContactForceOver100)
						});
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

				// DEBUG
				a_carCollider.wheels[wheelContact.index].contacts.back()[a_rk4Step].push_back(DebugContact{
					.carPoint = wheelContact.carPoint,
					.staticPoint = wheelContact.staticPoint,
					.force = contactForceOver100,
					.torque = glm::vec3{ 0.0f }
					});

			}

			if (suspensionState.length < 0.0f)
			{
				auto const springForceOver100 = suspensionState.length * wheel.compressedElasticityOver100 * wheelDown;
				auto const dampingCoefficient = computeDampingCoefficient(
					wheel.compressedRestitution, wheel.compressedElasticityOver100, wheel.mass);
				auto const damperForceOver100 = -dampingCoefficient * suspensionState.velocity * wheelDown / 100.0f;

				forceOver100 += springForceOver100 + damperForceOver100;
				torqueOver100 += glm::cross(suspensionCarLever, springForceOver100 + damperForceOver100);
				ignorable_assert(k_disableIgnorableAsserts || glm::length(torqueOver100) < 1'000'000.0f);

				wheelForceOver100 -= glm::dot(springForceOver100 + damperForceOver100, wheelDown);

				// DEBUG
				a_carCollider.wheels[wheelContact.index].chassisContacts.back()[a_rk4Step].push_back(DebugContact{
					.carPoint = suspensionAttachPosition,
					.staticPoint = wheelPosition,
					.force = springForceOver100 + damperForceOver100,
					.torque = glm::cross(suspensionCarLever, springForceOver100 + damperForceOver100)
				});

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
		ignorable_assert(k_disableIgnorableAsserts || glm::length(derivativeCarStateOver100.rotation) < 1'000'000.0f);
		auto const angularVelocity = rotationMatrix * a_carState.angularVelocityLocal;
		derivativeCarStateOver100.angularVelocityLocal =
			rotationMatrixInv * (inertiaInv * (torqueOver100 - glm::cross(angularVelocity, inertia * angularVelocity) / 100.0f));
		ignorable_assert(k_disableIgnorableAsserts || glm::length(derivativeCarStateOver100.angularVelocityLocal) < 1'000'000.0f);

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

		ignorable_assert(k_disableIgnorableAsserts || glm::length(angularVelocityWorld) > 0.000001f);

		glm::vec3 axis = angularVelocityWorld / glm::length(angularVelocityWorld);
		glm::quat delta = glm::angleAxis(angle, axis);
		ignorable_assert(k_disableIgnorableAsserts || glm::length(delta * a_rotation) > 0.000001f);
		return glm::normalize(delta * a_rotation);
	}

	glm::vec3 clampLinearVelocity(glm::vec3 const& a_linearVelocity)
	{
		using namespace misph::literals;
		static auto const k_maxLinearSpeed = (10'000_kmph).get_value();
		auto const absLinearVelocity = glm::length(a_linearVelocity);
		if (absLinearVelocity > k_maxLinearSpeed)
		{
			return a_linearVelocity * k_maxLinearSpeed / absLinearVelocity;
		}
		else
		{
			return a_linearVelocity;
		}
	}

	glm::vec3 clampAngularVelocityLocal(glm::vec3 const& a_angularVelocityLocal)
	{
		static auto const k_maxAngularVelocity = 2.0f * std::numbers::pi_v<float> *100.0f;
		auto const absAngularVelocityLocal = glm::length(a_angularVelocityLocal);
		if (absAngularVelocityLocal > k_maxAngularVelocity)
		{
			return a_angularVelocityLocal * k_maxAngularVelocity / absAngularVelocityLocal;
		}
		else
		{
			return a_angularVelocityLocal;
		}
	}

	float clampSuspensionVelocity(float a_suspensionVelocity)
	{
		static auto const k_maxSuspensionVelocity = 1000.0f;
		auto const absSuspensionVelocity = std::abs(a_suspensionVelocity);
		if (absSuspensionVelocity > k_maxSuspensionVelocity)
		{
			return a_suspensionVelocity * k_maxSuspensionVelocity / absSuspensionVelocity;
		}
		else
		{
			return a_suspensionVelocity;
		}
	}

	static Rk4CarState integrateCarState(Rk4CarState const& a_initialCarState, Rk4CarState const& a_derivativeCarStateOver100, float a_deltaTime)
	{
		Rk4CarState newCarState = a_initialCarState;
		newCarState.position += a_derivativeCarStateOver100.position * (100.0f * a_deltaTime);
		newCarState.rotation = integrateQuaterion(newCarState.rotation, a_derivativeCarStateOver100.rotation, 100.0f * a_deltaTime);

		newCarState.linearVelocity += a_derivativeCarStateOver100.linearVelocity * (100.0f * a_deltaTime);
		newCarState.linearVelocity = clampLinearVelocity(newCarState.linearVelocity);
		// TODO: is angular velocity integration accurate?
		newCarState.angularVelocityLocal += a_derivativeCarStateOver100.angularVelocityLocal * (100.0f * a_deltaTime);
		newCarState.angularVelocityLocal = clampAngularVelocityLocal(newCarState.angularVelocityLocal);

		for (int32_t i = 0; i < 4; ++i)
		{
			newCarState.suspensions[i].length += a_derivativeCarStateOver100.suspensions[i].length * (100.0f * a_deltaTime);
			newCarState.suspensions[i].velocity += a_derivativeCarStateOver100.suspensions[i].velocity * (100.0f * a_deltaTime);
			newCarState.suspensions[i].velocity = clampSuspensionVelocity(newCarState.suspensions[i].velocity);
		}

		return newCarState;
	}

	static void processNarrowPhaseStep(
		glm::vec3& a_carPosition,
		glm::quat& a_carRotation,
		CarCollider& a_carCollider,
		float a_duration,
		std::array<WheelGroundState, 4>& a_wheelGroundedStates,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates)
	{
		auto initialCarState = Rk4CarState{ a_carPosition, a_carRotation, a_carCollider.linearVelocity, a_carCollider.angularVelocityLocal };
		for (int32_t i = 0; i < 4; ++i)
		{
			initialCarState.suspensions[i] = { a_carCollider.wheels[i].suspensionLength, a_carCollider.wheels[i].suspensionVelocity };
		}

		auto const k1Over100 = computeDerivativeCarStateOver100(
			initialCarState, a_carCollider, a_broadPhaseCandidates, a_wheelGroundedStates, 0);
		auto const k2Over100 = computeDerivativeCarStateOver100(
			integrateCarState(initialCarState, k1Over100, a_duration / 2), a_carCollider, a_broadPhaseCandidates, a_wheelGroundedStates, 1);
		auto const k3Over100 = computeDerivativeCarStateOver100(
			integrateCarState(initialCarState, k2Over100, a_duration / 2), a_carCollider, a_broadPhaseCandidates, a_wheelGroundedStates, 2);
		auto const k4Over100 = computeDerivativeCarStateOver100(
			integrateCarState(initialCarState, k3Over100, a_duration), a_carCollider, a_broadPhaseCandidates, a_wheelGroundedStates, 3);

		if (k_immediateStop)
			return;

		a_carPosition += (100.0f * a_duration / 6.0f) * (k1Over100.position + 2.0f * k2Over100.position + 2.0f * k3Over100.position + k4Over100.position);
		a_carRotation += (100.0f * a_duration / 6.0f) * (k1Over100.rotation + 2.0f * k2Over100.rotation + 2.0f * k3Over100.rotation + k4Over100.rotation);
		a_carRotation = glm::normalize(a_carRotation);
		a_carCollider.linearVelocity += (100.0f * a_duration / 6.0f)
			* (k1Over100.linearVelocity + 2.0f * k2Over100.linearVelocity + 2.0f * k3Over100.linearVelocity + k4Over100.linearVelocity);
		a_carCollider.linearVelocity = clampLinearVelocity(a_carCollider.linearVelocity);
		// TODO: is angular velocity integration accurate?
		a_carCollider.angularVelocityLocal += (100.0f * a_duration / 6.0f)
			* (k1Over100.angularVelocityLocal + 2.0f * k2Over100.angularVelocityLocal + 2.0f * k3Over100.angularVelocityLocal + k4Over100.angularVelocityLocal);
		a_carCollider.angularVelocityLocal = clampAngularVelocityLocal(a_carCollider.angularVelocityLocal);

		for (int32_t i = 0; i < 4; ++i)
		{
			auto const& susK1Over100 = k1Over100.suspensions[i];
			auto const& susK2Over100 = k2Over100.suspensions[i];
			auto const& susK3Over100 = k3Over100.suspensions[i];
			auto const& susK4Over100 = k4Over100.suspensions[i];

			a_carCollider.wheels[i].suspensionLength += (100.0f * a_duration / 6.0f) * (susK1Over100.length + 2.0f * susK2Over100.length + 2.0f * susK3Over100.length + susK4Over100.length);
			a_carCollider.wheels[i].suspensionVelocity += (100.0f * a_duration / 6.0f) * (susK1Over100.velocity + 2.0f * susK2Over100.velocity + 2.0f * susK3Over100.velocity + susK4Over100.velocity);
			a_carCollider.wheels[i].suspensionVelocity = clampSuspensionVelocity(a_carCollider.wheels[i].suspensionVelocity);
		}
	}

	static void processNarrowUpdate(
		glm::vec3& a_carPosition,
		glm::quat& a_carRotation,
		CarCollider& a_carCollider,
		float a_duration,
		int32_t a_stepCount,
		std::vector<BroadPhaseCandidate> const& a_broadPhaseCandidates)
	{
		a_carCollider.narrowPhaseContacts.clear();

		std::array<WheelGroundState, 4> wheelGroundedStates;

		auto const stepDuration = a_duration / a_stepCount;
		for (int32_t stepIndex = 0; stepIndex < a_stepCount; ++stepIndex)
		{
			// DEBUG
			for (auto& chassisPart : a_carCollider.chassisParts)
			{
				chassisPart.contacts.emplace_back();
			}
			for (auto& wheel : a_carCollider.wheels)
			{
				wheel.contacts.emplace_back();
				wheel.chassisContacts.emplace_back();
			}

			processNarrowPhaseStep(a_carPosition, a_carRotation, a_carCollider, stepDuration, wheelGroundedStates, a_broadPhaseCandidates);
			if (k_immediateStop)
				break;
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
		auto /*const*/& fixedRateTimeContext = m_fixedRateTimeContext.get(a_wdap);
		auto& collisionContext = m_collisionContext.get(a_wdap);
		if (collisionContext.lastFixedTickIndexProcessed == fixedRateTimeContext.tickIndex)
		{
			return;
		}
		collisionContext.lastFixedTickIndexProcessed = fixedRateTimeContext.tickIndex;
		auto const tickDuration = std::chrono::duration<float>(fixedRateTimeContext.tickDuration).count();

		std::vector<BroadPhaseCandidate> broadPhaseCandidates;
		for (auto [carEntity, carPosition, carRotation, carCollider] : m_carColliderEntities.get(a_wdap).each())
		{
			// DEBUG
			for (auto& chassisPart : carCollider.chassisParts)
			{
				chassisPart.contacts.clear();
			}
			for (auto& wheel : carCollider.wheels)
			{
				wheel.contacts.clear();
				wheel.chassisContacts.clear();
			}

			collectBroadPhaseCandidates(
				carPosition, carRotation, carCollider, m_staticColliderEntities.get(a_wdap), broadPhaseCandidates);

			processNarrowUpdate(
				carPosition, carRotation, carCollider, tickDuration, collisionContext.updateStepCount, broadPhaseCandidates);
		}

		if (k_immediateStop)
		{
			k_immediateStop = false;
			fixedRateTimeContext.debugRemainingTickCount = 0;
		}
	}
}
