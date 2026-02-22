#pragma once

#include <vob/aoe/physics/Material.h>
#include <vob/aoe/physics/Shapes.h>

#include <glm/ext/quaternion_float.hpp>
#include <glm/glm.hpp>

#include <array>
#include <numbers>
#include <vector>


namespace vob::aoeph
{
	struct StaticCollider
	{
		struct Part
		{
			Material material;
			std::vector<Triangle> triangles;
		};

		std::vector<Part> parts;

		Aabb bounds;
	};

	struct CarCollider
	{
		struct ChassisPart
		{
			// Physical properties
			glm::vec3 position = glm::vec3{ 0.0f };
			glm::quat rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
			glm::vec3 radiuses = glm::vec3{ 1.0f };
			Material material = Material{ 800'000.0f, 0.01f, 0.5f };
		};

		struct Wheel
		{
			// Physical properties
			glm::vec3 suspensionAttachPosition = glm::vec3{ 0.0f };
			glm::quat rotation = glm::quat();
			glm::vec3 radiuses = glm::vec3{ 1.0f };
			Material rimMaterial = Material{ 800'000.0f, 0.01f, 0.5f };
			Material tireMaterial = Material{ 200'000.0f, 0.01f, 0.5f };

			float suspensionRestLength = 0.3f;
			float suspensionMaxLength = 0.2f;
			float suspensionElasticityOver100 = 216.0f;
			float suspensionDamperOver100 = 216.0f;
			float compressedElasticityOver100 = 800'000.0f;
			float compressedRestitution = 0.1f;
			float tireMaxAngle = std::numbers::pi_v<float> / 4.0f;
			float mass = 10.0f;

			// Runtime state
			float steeringAngle = 0.0f;
			float suspensionLength = 0.0f;
			float suspensionVelocity = 0.0f;
			bool isGrounded = false;
			glm::vec3 groundNormal = glm::vec3{ 0.0f };
			glm::vec3 groundPoint = glm::vec3{ 0.0f };
			glm::vec3 tireGroundedPoint = glm::vec3{ 0.0f };
			Material groundMaterial;
		};

		// Properties
		std::vector<ChassisPart> chassisParts;
		std::array<Wheel, 4> wheels;

		float mass = 1000.0f;
		glm::mat3 inertiaLocal = glm::mat3{ 1.0f };
		glm::vec3 barycenterLocal = glm::vec3{ 0.0f };
		glm::vec3 boundsCenterLocal = glm::vec3{ 0.0f };
		glm::vec3 boundsHalfExtentsLocal = glm::vec3{ 0.0f };

		// Runtime state
		glm::vec3 force = glm::vec3{ 0.0f };
		glm::vec3 torque = glm::vec3{ 0.0f };
		glm::vec3 linearVelocity = glm::vec3{ 0.0f };
		glm::vec3 angularVelocityLocal = glm::vec3{ 0.0f };
	};
}
