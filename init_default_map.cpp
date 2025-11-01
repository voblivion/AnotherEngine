#include "init_default_map.h"

#include <vob/aoe/debug/controllable_tag.h>
#include <vob/aoe/debug/controlled_tag.h>
#include <vob/aoe/debug/debug_controller.h>
#include <vob/aoe/debug/debug_ghost_controller_component.h>
#include <vob/aoe/engine/world.h>
#include <vob/aoe/input/binding_util.h>
#include <vob/aoe/input/bindings.h>
#include <vob/aoe/rendering/components/model_data_component.h>
#include <vob/aoe/rendering/world_components/director_world_component.h>
#include <vob/aoe/spacetime/transform.h>
#include <vob/aoe/spacetime/soft_follow.h>

#include "DataHolder.h"

// WIP vehicle / wheels
#include <vob/aoe/physics/components/rigidbody.h>
#include <vob/aoe/physics/vehicle_physics_component.h>
#include <vob/aoe/physics/components/car_controller.h>
#include <bullet/BulletCollision/CollisionShapes/btCylinderShape.h>
#include <bullet/BulletCollision/CollisionShapes/btBoxShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCompoundShape.h>
#include <bullet/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <bullet/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>
#include <bullet/BulletDynamics/ConstraintSolver/btHinge2Constraint.h>


std::vector<vob::aoeph::triangle> transform_surface(glm::vec3 const& a_position, glm::quat const& a_rotation, std::vector<vob::aoeph::triangle> const& a_triangles)
{
	std::vector<vob::aoeph::triangle> result;
	for (auto& tr : a_triangles)
	{
		result.emplace_back(
			a_position + a_rotation * tr.p0,
			a_position + a_rotation * tr.p1,
			a_position + a_rotation * tr.p2);
	}
	return result;
}

std::vector<vob::aoeph::triangle> create_flat_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	std::vector<vob::aoeph::triangle> triangles;
	triangles.emplace_back(
		glm::vec3{ 32.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 2.0f, 32.0f }
	);
	triangles.emplace_back(
		glm::vec3{ 32.0f, 2.0f, 32.0f },
		glm::vec3{ 32.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 2.0f, 32.0f }
	);
	triangles.emplace_back(
		glm::vec3{0.0f, 2.0f, 0.0f},
		glm::vec3{0.0f, 0.0f, 0.0f},
		glm::vec3{0.0f, 2.0f, 32.0f}
	);
	triangles.emplace_back(
		glm::vec3{ 0.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 2.0f, 32.0f }
	);
	triangles.emplace_back(
		glm::vec3{ 0.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 0.0f, 32.0f },
		glm::vec3{ 0.0f, 2.0f, 32.0f }
	);
	triangles.emplace_back(
		glm::vec3{ 32.0f, 2.0f, 0.0f },
		glm::vec3{ 32.0f, 2.0f, 32.0f },
		glm::vec3{ 32.0f, 0.0f, 0.0f }
	);
	triangles.emplace_back(
		glm::vec3{ 32.0f, 2.0f, 32.0f },
		glm::vec3{ 32.0f, 0.0f, 32.0f },
		glm::vec3{ 32.0f, 0.0f, 0.0f }
	);
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> create_slope_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	std::vector<vob::aoeph::triangle> triangles;
	triangles.emplace_back(
		glm::vec3{ 32.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 18.0f, 32.0f }
	);
	triangles.emplace_back(
		glm::vec3{ 32.0f, 18.0f, 32.0f },
		glm::vec3{ 32.0f, 2.0f, 0.0f },
		glm::vec3{ 0.0f, 18.0f, 32.0f }
	);
	return transform_surface(a_position, a_rotation, triangles);
}

std::pair<float, float> bezier_cubic(float y0, float x1, float y1, float x2, float y2, float x3, float y3, float t)
{
	auto const u = (1.0f - t);
	auto const x = u * u * u * 0.f + 3.0f * u * u * t * x1 + 3.0f * u * t * t * x2 + t * t * t * x3;
	auto const y = u * u * u * y0 + 3.0f * u * u * t * y1 + 3.0f * u * t * t * y2 + t * t * t * y3;
	return std::make_pair(x, y);
}

std::vector<vob::aoeph::triangle> create_smooth_step_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	constexpr int32_t k_subdivisions = 32;

	auto p = [](int32_t s, int32_t subdivisions) {
		return bezier_cubic(0.0f, 16.0f, 0.0f, 16.0f, 8.0f, 32.0f, 8.0f, static_cast<float>(s) / k_subdivisions);
	};

	std::vector<vob::aoeph::triangle> triangles;
	for (int32_t s = 0; s < k_subdivisions; ++s)
	{
		auto const [x0, y0] = p(s, k_subdivisions);
		auto const [x1, y1] = p(s + 1, k_subdivisions);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y1, x1 },
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
	}
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> create_smooth_step2_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	constexpr int32_t k_subdivisions = 32;

	auto p = [](int32_t s, int32_t subdivisions) {
		return bezier_cubic(0.0f, 16.0f, 0.0f, 16.0f, 16.0f, 32.0f, 16.0f, static_cast<float>(s) / k_subdivisions);
		};

	std::vector<vob::aoeph::triangle> triangles;
	for (int32_t s = 0; s < k_subdivisions; ++s)
	{
		auto const [x0, y0] = p(s, k_subdivisions);
		auto const [x1, y1] = p(s + 1, k_subdivisions);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y1, x1 },
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
	}
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> create_smooth_start_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	constexpr int32_t k_subdivisions = 32;

	auto p = [](int32_t s, int32_t subdivisions) {
		return bezier_cubic(0.0f, 16.0f, 0.0f, 16.0f, 0.0f, 32.0f, 8.0f, static_cast<float>(s) / k_subdivisions);
		};

	std::vector<vob::aoeph::triangle> triangles;
	for (int32_t s = 0; s < k_subdivisions; ++s)
	{
		auto const [x0, y0] = p(s, k_subdivisions);
		auto const [x1, y1] = p(s + 1, k_subdivisions);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y1, x1 },
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
	}
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> create_smooth_stop_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	constexpr int32_t k_subdivisions = 32;

	auto p = [](int32_t s, int32_t subdivisions) {
		return bezier_cubic(0.0f, 16.0f, 8.0f, 16.0f, 8.0f, 32.0f, 8.0f, static_cast<float>(s) / k_subdivisions);
		};

	std::vector<vob::aoeph::triangle> triangles;
	for (int32_t s = 0; s < k_subdivisions; ++s)
	{
		auto const [x0, y0] = p(s, k_subdivisions);
		auto const [x1, y1] = p(s + 1, k_subdivisions);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
		triangles.emplace_back(
			glm::vec3{ 32.0f, 2.0f + y1, x1 },
			glm::vec3{ 32.0f, 2.0f + y0, x0 },
			glm::vec3{ 0.0f, 2.0f + y1, x1 }
		);
	}
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> _create_turn_surface(glm::vec3 const& a_position, glm::quat const& a_rotation, int32_t a_size)
{
	constexpr int32_t k_subdivisions = 32;

	auto const minR = 32.0f * a_size;
	auto const maxR = 32.0f * (a_size + 1);

	std::vector<vob::aoeph::triangle> triangles;
	for (int32_t s = 0; s < k_subdivisions; ++s)
	{
		auto const r0 = 3.141592f / 2.0f * static_cast<float>(s) / k_subdivisions;
		auto const r1 = 3.141592f / 2.0f * static_cast<float>(s + 1) / k_subdivisions;
		triangles.emplace_back(
			glm::vec3{ maxR - minR * std::cos(r0), 2.0f, minR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r0), 2.0f, maxR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r1), 2.0f, maxR * std::sin(r1) }
		);
		triangles.emplace_back(
			glm::vec3{ maxR - minR * std::cos(r1), 2.0f, minR * std::sin(r1) },
			glm::vec3{ maxR - minR * std::cos(r0), 2.0f, minR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r1), 2.0f, maxR * std::sin(r1) }
		);
		triangles.emplace_back(
			glm::vec3{ maxR - maxR * std::cos(r0), 2.0f, maxR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r0), 0.0f, maxR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r1), 2.0f, maxR * std::sin(r1) });
		triangles.emplace_back(
			glm::vec3{ maxR - maxR * std::cos(r0), 0.0f, maxR * std::sin(r0) },
			glm::vec3{ maxR - maxR * std::cos(r1), 0.0f, maxR * std::sin(r1) },
			glm::vec3{ maxR - maxR * std::cos(r1), 2.0f, maxR * std::sin(r1) });
		triangles.emplace_back(
			glm::vec3{ maxR - minR * std::cos(r1), 2.0f, minR * std::sin(r1) },
			glm::vec3{ maxR - minR * std::cos(r1), 0.0f, minR * std::sin(r1) },
			glm::vec3{ maxR - minR * std::cos(r0), 2.0f, minR * std::sin(r0) }
		);
		triangles.emplace_back(
			glm::vec3{ maxR - minR * std::cos(r1), 0.0f, minR * std::sin(r1) },
			glm::vec3{ maxR - minR * std::cos(r0), 0.0f, minR * std::sin(r0) },
			glm::vec3{ maxR - minR * std::cos(r0), 2.0f, minR * std::sin(r0) }
		);
	}
	return transform_surface(a_position, a_rotation, triangles);
}

std::vector<vob::aoeph::triangle> create_turn0_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	return _create_turn_surface(a_position, a_rotation, 0);
}

std::vector<vob::aoeph::triangle> create_turn_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	return _create_turn_surface(a_position, a_rotation, 1);
}

std::vector<vob::aoeph::triangle> create_turn2_surface(glm::vec3 const& a_position, glm::quat const& a_rotation)
{
	return _create_turn_surface(a_position, a_rotation, 2);
}

void init_default_map(vob::aoeng::world& a_world, vob::aoe::DataHolder& a_data)
{
	auto& worldData = a_world.get_data();
	auto& entityRegistry = worldData.get_entity_registry();

	//auto const debugPawn = entityRegistry.create();
	//auto const debugCamera = entityRegistry.create();
	//{
	//	entityRegistry.emplace<vob::aoest::position>(debugPawn, 0.0f, 2.0f, 0.0f);
	//	entityRegistry.emplace<vob::aoest::rotation>(debugPawn);
	//	entityRegistry.emplace<vob::aoedb::controllable_tag>(debugPawn);
	//	entityRegistry.emplace<vob::aoedb::controlled_tag>(debugPawn);

	//	auto& debugController = entityRegistry.emplace<vob::aoedb::debug_controller_component>(
	//		debugPawn,
	//		0.0f /* camera pitch */,
	//		8.0f /* camera distance */,
	//		glm::vec3{ 0.0f, 2.0f, 0.0f } /* head offset */,
	//		debugCamera);

	//	auto sphereModelId = a_data.filesystemIndexer.get_runtime_id("data/new/models/capsule.gltf");
	//	entityRegistry.emplace<vob::aoegl::model_data_component>(
	//		debugPawn,
	//		a_data.modelDatabase.find(sphereModelId));
	//}
	//{
	//	entityRegistry.emplace<vob::aoest::position>(debugCamera, 0.0f, 0.0f, 0.0f);
	//	entityRegistry.emplace<vob::aoest::rotation>(debugCamera);

	//	entityRegistry.emplace<vob::aoegl::camera_component>(
	//		debugCamera);

	//	auto const cameraOffset = glm::vec3{ 0.0f, 4.0f, 8.0f };
	//	auto const cameraTarget = glm::vec3{ 0.0f, 2.0f, 0.0f };
	//	auto offsetRotation = glm::quatLookAt(glm::normalize(cameraTarget - cameraOffset), glm::vec3{ 0.0f, 1.0f, 0.0f });
	//	auto offsetMatrix = aoest::combine(cameraOffset, offsetRotation);
	//	entityRegistry.emplace<vob::aoest::attachment_component>(
	//		debugCamera,
	//		false /* needs update */,
	//		debugPawn,
	//		offsetMatrix);
	//}
	auto& bindings = a_world.get_world_component<vob::aoein::bindings>();

	//auto const track = entityRegistry.create();
	//{
	//	entityRegistry.emplace<vob::aoest::position>(track, 0.0f, 0.0f, 0.0f);
	//	entityRegistry.emplace<vob::aoest::rotation>(track);

	//	auto trackModelId = a_data.filesystemIndexer.get_runtime_id("data/new/models/Track.gltf");
	//	entityRegistry.emplace<vob::aoegl::model_data_component>(
	//		track,
	//		a_data.modelDatabase.find(trackModelId));

	//	// aya .. mem leak and all, disgusting!
	//	auto model = a_data.modelDatabase.find(trackModelId);
	//	auto& mesh = model->m_texturedMeshes[0].m_mesh;
	//	btTriangleIndexVertexArray* vertexArray = new btTriangleIndexVertexArray();
	//	btIndexedMesh indexedMesh;
	//	indexedMesh.m_numTriangles = static_cast<int>(mesh.m_triangles.size());
	//	indexedMesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(&(mesh.m_triangles[0].m_v0));
	//	indexedMesh.m_triangleIndexStride = sizeof(vob::aoegl::triangle);
	//	indexedMesh.m_numVertices = static_cast<int>(mesh.m_positions.size());
	//	indexedMesh.m_vertexBase = reinterpret_cast<const unsigned char*>(&(mesh.m_positions[0].x));
	//	indexedMesh.m_vertexStride = sizeof(glm::vec3);
	//	vertexArray->addIndexedMesh(indexedMesh);
	//	entityRegistry.emplace<vob::aoeph::rigidbody>(
	//		track,
	//		true,
	//		0.0f,
	//		glm::mat4{1.0f},
	//		std::make_shared<btBvhTriangleMeshShape>(vertexArray, true),
	//		std::make_shared<aoeph::material>());
	//}

	auto const ghostControlledCamera = entityRegistry.create();
	{

		entityRegistry.emplace<vob::aoest::position>(ghostControlledCamera, 0.0f, 0.0f, -5.0f);
		entityRegistry.emplace<vob::aoest::rotation>(ghostControlledCamera, glm::vec3{ 0.0f, -3.141592f, 0.0f });

		auto& ghostController = entityRegistry.emplace<vob::aoedb::debug_ghost_controller_component>(ghostControlledCamera);
		ghostController.m_lateralMoveMapping = bindings.axes.add(aoein::binding_util::make_axis(
			aoein::keyboard::key::S, aoein::keyboard::key::F));
		ghostController.m_longitudinalMoveMapping = bindings.axes.add(aoein::binding_util::make_axis(
			aoein::keyboard::key::E, aoein::keyboard::key::D));
		ghostController.m_verticalMoveMapping = bindings.axes.add(aoein::binding_util::make_axis(
			aoein::keyboard::key::Space, aoein::keyboard::key::LBracket));
		ghostController.m_yawMapping = bindings.axes.add(aoein::binding_util::make_derived_axis(
			aoein::mouse::axis::X, 0.001f));
		ghostController.m_pitchMapping = bindings.axes.add(aoein::binding_util::make_derived_axis(
			aoein::mouse::axis::Y, 0.001f));
		ghostController.m_enableViewMapping = bindings.switches.add(aoein::binding_util::make_switch(
			aoein::mouse::button::Right));
		ghostController.m_decreaseSpeedMapping = bindings.switches.add(aoein::binding_util::make_switch(
			aoein::mouse::button::ScrollDown));
		ghostController.m_increaseSpeedMapping = bindings.switches.add(aoein::binding_util::make_switch(
			aoein::mouse::button::ScrollUp));

		entityRegistry.emplace<vob::aoegl::camera_component>(ghostControlledCamera);

		entityRegistry.emplace<vob::aoedb::controllable_tag>(ghostControlledCamera);
		entityRegistry.emplace<vob::aoedb::controlled_tag>(ghostControlledCamera);
	}

	auto const dynBody = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(dynBody, 0.0f, 3.0f, 100.0f);
		entityRegistry.emplace<vob::aoest::rotation>(dynBody);
		entityRegistry.emplace<vob::aoeph::linear_velocity>(dynBody);
		entityRegistry.emplace<vob::aoeph::angular_velocity_local>(dynBody);

		auto& carCollider = entityRegistry.emplace<vob::aoeph::car_collider>(dynBody);
		// front axel
		carCollider.chassisParts.emplace_back(glm::vec3{ -0.01553f, 0.36325f, -1.75357f }, glm::quat{ glm::vec3{0.0f} }, glm::vec3{ 0.905f, 0.283f, 0.385f });
		// mid axel
		carCollider.chassisParts.emplace_back(glm::vec3{ 0.0f, 0.471f, -0.219f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.439f, 0.362f, 1.902f });
		// cockpit
		carCollider.chassisParts.emplace_back(glm::vec3{ 0.0f, 0.65281f, 0.89763f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 1.021f, 0.515f, 1.038f });
		// chassis
		carCollider.chassisParts.emplace_back(glm::vec3{ 0.0f, 0.44878f, 0.20792f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.968f, 0.363f, 1.682f });

		// front left wheel
		carCollider.wheels.emplace_back(glm::vec3{ -0.86301f, 0.3525f, -1.78209f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }).turnFactor = 1.0f;
		// front right wheel
		carCollider.wheels.emplace_back(glm::vec3{ 0.86299f, 0.3525f, -1.78209f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }).turnFactor = 1.0f;
		// rear left wheel
		carCollider.wheels.emplace_back(glm::vec3{ -0.885f, 0.3525f, 1.2055f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f });
		// rear right wheel
		carCollider.wheels.emplace_back(glm::vec3{ 0.885f, 0.3525f, 1.2055f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f });

		carCollider.mass = 1'500.0f;
		carCollider.barycenter = glm::vec3{ 0.0f, 0.0f, -0.288295f };
		carCollider.inertia = glm::mat3{ carCollider.mass / 5.0f };
		carCollider.inertia[0][0] *= (0.7f * 0.7f + 1.6f * 1.6f);
		carCollider.inertia[1][1] *= (1.6f * 1.6f + 0.9f * 0.9f);
		carCollider.inertia[2][2] *= (0.9f * 0.9f + 0.7f * 0.7f);

		auto bounds = aoeph::aabb{ glm::vec3{ std::numeric_limits<float>::max() }, glm::vec3{ -std::numeric_limits<float>::max() } };

		for (auto const& chassisPart : carCollider.chassisParts)
		{
			auto const matrix = glm::mat3(glm::quat{ chassisPart.rotation });
			auto partExtents =
				glm::abs(matrix[0]) * chassisPart.radiuses +
				glm::abs(matrix[1]) * chassisPart.radiuses +
				glm::abs(matrix[2]) * chassisPart.radiuses;

			bounds.min = glm::min(bounds.min, chassisPart.position - partExtents);
			bounds.max = glm::max(bounds.max, chassisPart.position + partExtents);
		}

		for (auto const& wheel : carCollider.wheels)
		{

			auto const matrix = glm::mat3(glm::quat{ wheel.rotation });
			auto partExtents =
				glm::abs(matrix[0]) * wheel.radiuses +
				glm::abs(matrix[1]) * wheel.radiuses +
				glm::abs(matrix[2]) * wheel.radiuses;

			auto const wheelHighPosition = wheel.attachPosition;
			bounds.min = glm::min(bounds.min, wheelHighPosition - partExtents);
			bounds.max = glm::max(bounds.max, wheelHighPosition + partExtents);
			auto const wheelLowPosition = wheelHighPosition - wheel.rotation * glm::vec3{ 0.0f, -wheel.suspensionMaxLength, 0.0f };
			bounds.min = glm::min(bounds.min, wheelLowPosition - partExtents);
			bounds.max = glm::max(bounds.max, wheelLowPosition + partExtents);
		}

		// TODO: this 2.778f should be added at search time, whatever
		carCollider.boundsLocal = aoeph::aabb{ bounds.min - glm::vec3{2.778f}, bounds.max + glm::vec3{2.778f} };

		/*
		auto& dynamicBody = entityRegistry.emplace<vob::aoeph::dynamic_body>(dynBody);
		// front axel
		dynamicBody.parts.emplace_back(glm::vec3{ -0.01553f, 0.36325f, -1.75357f }, glm::quat{ glm::vec3{0.0f} }, glm::vec3{ 0.905f, 0.283f, 0.385f });
		// mid axel
		dynamicBody.parts.emplace_back(glm::vec3{ 0.0f, 0.471f, -0.219f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{0.439f, 0.362f, 1.902f});
		// cockpit
		dynamicBody.parts.emplace_back(glm::vec3{ 0.0f, 0.65281f, 0.89763f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 1.021f, 0.515f, 1.038f });
		// chassis
		dynamicBody.parts.emplace_back(glm::vec3{ 0.0f, 0.44878f, 0.20792f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.968f, 0.363f, 1.682f });
		// front left wheel
		dynamicBody.parts.emplace_back(glm::vec3{ -0.86301f, 0.3525f, -1.78209f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }).debug_draw_enabled = false;
		// front right wheel
		dynamicBody.parts.emplace_back(glm::vec3{ 0.86299f, 0.3525f, -1.78209f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }).debug_draw_enabled = false;
		// rear left wheel
		dynamicBody.parts.emplace_back(glm::vec3{ -0.885f, 0.3525f, 1.2055f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }).debug_draw_enabled = false;
		// rear right wheel
		dynamicBody.parts.emplace_back(glm::vec3{ 0.885f, 0.3525f, 1.2055f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }).debug_draw_enabled = false;
		dynamicBody.barycenter = glm::vec3{ 0.0f, 0.0f, -.288295f };

		dynamicBody.mass = 1'000.0f;
		dynamicBody.inertia = glm::mat3{ dynamicBody.mass / 5.0f };
		dynamicBody.inertia[0][0] *= (0.7f * 0.7f + 1.6f * 1.6f);
		dynamicBody.inertia[1][1] *= (1.6f * 1.6f + 0.9f * 0.9f);
		dynamicBody.inertia[2][2] *= (0.9f * 0.9f + 0.7f * 0.7f);*/

		/*auto& vehiclePhysicsCmp = entityRegistry.emplace<vob::aoeph::vehicle_physics_component>(dynBody);
		// front left
		vehiclePhysicsCmp.wheels.emplace_back(glm::vec3{ -0.86301f, 0.3525f, -1.78209f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }, 1.f);
		vehiclePhysicsCmp.wheels.emplace_back(glm::vec3{ 0.86301f, 0.3525f, -1.78209f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }, 1.f);
		vehiclePhysicsCmp.wheels.emplace_back(glm::vec3{ -0.885f, 0.3525f, 1.2055f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }, 0.f);
		vehiclePhysicsCmp.wheels.emplace_back(glm::vec3{ 0.885f, 0.3525f, 1.2055f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{ 0.182f, 0.364f, 0.364f }, 0.f);*/
	}

	auto const staBody = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(staBody, 0.0f, 0.0f, 0.0f);
		entityRegistry.emplace<vob::aoest::rotation>(staBody);
		auto& staticCollider = entityRegistry.emplace<vob::aoeph::static_collider>(staBody);
		auto& part = staticCollider.parts.emplace_back(vob::aoeph::material{});
		part.triangles.emplace_back(vob::aoeph::triangle{
			glm::vec3{-5.0f, 2.0f, -5.0f},
			glm::vec3{-5.0f, 2.0f, 200.0f},
			glm::vec3{200.0f, 2.0f, -5.0f}
		});
		part.triangles.emplace_back(vob::aoeph::triangle{
			glm::vec3{200.0f, 2.0f, -5.0f},
			glm::vec3{-5.0f, 2.0f, 200},
			glm::vec3{200.0f, -2.0f, -5.0f}
		});
		part.triangles.emplace_back(vob::aoeph::triangle{
			glm::vec3{-5.0f, 2.0f, -5.0f},
			glm::vec3{-200.0f, 20.0f, -5.0f},
			glm::vec3{-5.0f, 2.0f, 200.0f}
		});
		part.triangles.emplace_back(vob::aoeph::triangle{
			glm::vec3{-200.0f, 2.0f, -5.0f},
			glm::vec3{200.0f, 2.0f, -5.0f},
			glm::vec3{0.0f, 2.0f, -200.0f}
		});
		staticCollider.bounds = vob::aoeph::aabb{ glm::vec3{-500.0f, -2.0f, -500.0f}, glm::vec3{500.0f, 264.0f, 500.0f} };
		
		/*
		// Looping
		auto const width = 16.0f;
		auto const radius = 16.0f;
		auto const segments = 48;
		for (int i = 0; i < segments; ++i)
		{
			auto const r0 = static_cast<float>(i + 0) / segments;
			auto const r1 = static_cast<float>(i + 1) / segments;

			auto const x00 = -5.0f + r0 * width;
			auto const x01 = x00 + width;
			auto const x10 = -5.0f + r1 * width;
			auto const x11 = x10 + width;
			auto const a0 = 2.0f * 3.141592f * r0;
			auto const a1 = 2.0f * 3.141592f * r1;
			auto const z0 = radius - radius * std::sin(a0);
			auto const y0 = 2.0f + radius - radius * std::cos(a0);
			auto const z1 = radius - radius * std::sin(a1);
			auto const y1 = 2.0f + radius - radius * std::cos(a1);

			part.triangles.emplace_back(vob::aoeph::triangle{
				glm::vec3{x00, y0, z0},
				glm::vec3{x01, y0, z0},
				glm::vec3{x10, y1, z1}});

			part.triangles.emplace_back(vob::aoeph::triangle{
				glm::vec3{x10, y1, z1},
				glm::vec3{x01, y0, z0},
				glm::vec3{x11, y1, z1}});
		}

		// Half pipes
		for (int k = 0; k < 2; ++k)
		{
			auto lastX0 = 0.0f;
			auto lastX1 = 0.0f;
			auto lastY1 = 0.0f;
			auto lastZ1 = 0.0f;
			for (int i = 0; i < segments * 2 / 4; ++i)
			{
				auto const r0 = static_cast<float>(i + 0) / segments / 2;
				auto const r1 = static_cast<float>(i + 1) / segments / 2;

				auto const x0 = 45.0f + k * (2 * width);
				auto const x1 = x0 + width;
				auto const a0 = 2.0f * 3.141592f * r0;
				auto const a1 = 2.0f * 3.141592f * r1;
				auto const z0 = radius - radius * std::sin(a0);
				auto const y0 = 2.0f + radius - radius * std::cos(a0);
				auto const z1 = radius - radius * std::sin(a1);
				auto const y1 = 2.0f + radius - radius * std::cos(a1);

				lastX0 = x0;
				lastX1 = x1;
				lastY1 = y1;
				lastZ1 = z1;

				part.triangles.emplace_back(vob::aoeph::triangle{
					glm::vec3{x0, y0, z0},
					glm::vec3{x1, y0, z0},
					glm::vec3{x0, y1, z1} });

				part.triangles.emplace_back(vob::aoeph::triangle{
					glm::vec3{x0, y1, z1},
					glm::vec3{x1, y0, z0},
					glm::vec3{x1, y1, z1} });

				// side walls
				part.triangles.emplace_back(vob::aoeph::triangle{
					glm::vec3{x0, y0, z0},
					glm::vec3{x0, y1, z1},
					glm::vec3{x0, 2.0f, z1} });
				part.triangles.emplace_back(vob::aoeph::triangle{
					glm::vec3{x1, y0, z0},
					glm::vec3{x1, 2.0f, z1},
					glm::vec3{x1, y1, z1} });
				if (i != 0)
				{
					part.triangles.emplace_back(vob::aoeph::triangle{
						glm::vec3{x0, 2.0f, z1},
						glm::vec3{x0, 2.0f, z0},
						glm::vec3{x0, y0, z0} });
					part.triangles.emplace_back(vob::aoeph::triangle{
						glm::vec3{x1, 2.0f, z1},
						glm::vec3{x1, y0, z0},
						glm::vec3{x1, 2.0f, z0} });
				}
			}

			// back wall
			part.triangles.emplace_back(vob::aoeph::triangle{
				glm::vec3{lastX1, 2.0f, lastZ1},
				glm::vec3{lastX0, 2.0f, lastZ1},
				glm::vec3{lastX0, lastY1, lastZ1} });
			part.triangles.emplace_back(vob::aoeph::triangle{
				glm::vec3{lastX1, 2.0f, lastZ1},
				glm::vec3{lastX0, lastY1, lastZ1},
				glm::vec3{lastX1, lastY1, lastZ1} });
			// top wall
			part.triangles.emplace_back(vob::aoeph::triangle{
				glm::vec3{lastX0, lastY1, lastZ1},
				glm::vec3{lastX1, lastY1, lastZ1},
				glm::vec3{lastX0, lastY1 + 4.0f, lastZ1} });
			part.triangles.emplace_back(vob::aoeph::triangle{
				glm::vec3{lastX0, lastY1 + 4.0f, lastZ1},
				glm::vec3{lastX1, lastY1, lastZ1},
				glm::vec3{lastX1, lastY1 + 4.0f, lastZ1} });
		}

		// some wall
		part.triangles.emplace_back(vob::aoeph::triangle{
			glm::vec3{120.0f, 2.0f, 5.0f},
			glm::vec3{120.0f, 2.0f, 50.0f},
			glm::vec3{120.0f, 20.0f, 5.0f}
			});
		part.triangles.emplace_back(vob::aoeph::triangle{
			glm::vec3{120.0f, 2.0f, 50.0f},
			glm::vec3{120.0f, 20.0f, 50.0f},
			glm::vec3{120.0f, 20.0f, 5.0f}
			});

		*/

		auto addPart = [&entityRegistry](auto const a_type, float x, float y, float z, float r)
			{
				auto const staticEntity = entityRegistry.create();
				auto& staticCollider = entityRegistry.emplace<vob::aoeph::static_collider>(staticEntity);
				auto& staticPart = staticCollider.parts.emplace_back(vob::aoeph::material{});

				auto& pos = entityRegistry.emplace<vob::aoest::position>(staticEntity, glm::vec3{ x * 32.0f, y * 8.0f, 100.0f + z * 32.0f });
				auto& rot = entityRegistry.emplace<vob::aoest::rotation>(staticEntity, glm::angleAxis(r * 3.141592f / 2.0f, glm::vec3{ 0.0f, 1.0f, 0.0f }));
				staticPart.triangles = a_type(glm::vec3{ 0.0f }, glm::quat(glm::vec3{ 0.0f }));

				staticCollider.bounds = aoeph::aabb{ glm::vec3{ std::numeric_limits<float>::max() }, glm::vec3{ -std::numeric_limits<float>::max() } };
				for (auto& triangle : staticPart.triangles)
				{
					staticCollider.bounds.min = glm::min(staticCollider.bounds.min, pos + rot * triangle.p0);
					staticCollider.bounds.min = glm::min(staticCollider.bounds.min, pos + rot * triangle.p1);
					staticCollider.bounds.min = glm::min(staticCollider.bounds.min, pos + rot * triangle.p2);
					staticCollider.bounds.max = glm::max(staticCollider.bounds.max, pos + rot * triangle.p0);
					staticCollider.bounds.max = glm::max(staticCollider.bounds.max, pos + rot * triangle.p1);
					staticCollider.bounds.max = glm::max(staticCollider.bounds.max, pos + rot * triangle.p2);
				}

				// auto& part0 = staticCollider.parts.emplace_back(vob::aoeph::material{});
				// part0.triangles = a_type(glm::vec3{ x * 32.0f, y * 8.0f, 100.0f + z * 32.0f }, );
			};

		addPart(create_flat_surface, 0, 0, 0, 0);
		addPart(create_smooth_start_surface, 0, 0, 1, 0);
		addPart(create_slope_surface, 0, 1, 2, 0);
		addPart(create_smooth_stop_surface, 0, 3, 3, 0);
		addPart(create_flat_surface, 0, 4, 4, 0);
		addPart(create_flat_surface, 0, 4, 5, 0);
		addPart(create_flat_surface, 0, 4, 6, 0);
		addPart(create_flat_surface, 0, 4, 7, 0);
		addPart(create_turn_surface, 0, 4, 8, 0);
		addPart(create_turn2_surface, 2, 4, 10, 1);
		addPart(create_turn_surface, 6, 4, 5, -1);
		addPart(create_smooth_step2_surface, 7, 2, 5, -1);
		addPart(create_turn_surface, 7, 2, 6, 1);
		addPart(create_smooth_step_surface, 8, 1, 3, 0);
		addPart(create_smooth_step_surface, 9, 1, 3, 0);
		addPart(create_flat_surface, 8, 1, 2, 0);
		addPart(create_flat_surface, 9, 1, 2, 0);
		addPart(create_flat_surface, 8, 1, 1, 0);
		addPart(create_flat_surface, 9, 1, 1, 0);
		addPart(create_flat_surface, 8, 1, 0, 0);
		addPart(create_flat_surface, 9, 1, 0, 0);
		addPart(create_flat_surface, 8, 0, -1, 0);
		addPart(create_flat_surface, 9, 0, -1, 0);
		addPart(create_turn2_surface, 9, 0, -1, 2);
		addPart(create_flat_surface, 5, 0, -4, 0);


		addPart(create_smooth_step2_surface, 0, 0, -7, 0);
		addPart(create_flat_surface, 0, 2, -6, 0);
		addPart(create_smooth_step2_surface, 1, 0, -7, 1);
		addPart(create_flat_surface, 2, 2, -8, 0);
		addPart(create_smooth_step2_surface, 1, 0, -8, 2);
		addPart(create_flat_surface, 0, 2, -10, 0);
		addPart(create_slope_surface, 0, 0, -8, 3);
		addPart(create_flat_surface, -2, 2, -8, 0);

		addPart(create_smooth_step2_surface, 3, 0, -5, 1);
		addPart(create_smooth_step2_surface, 3, 0, -6, 1);
		addPart(create_smooth_step2_surface, 3, 0, -6, 2);
		addPart(create_smooth_step2_surface, 3, 0, -6, 3);
		addPart(create_smooth_step2_surface, 3, 0, -7, 3);
		addPart(create_smooth_step2_surface, 4, 0, -6, 2);

		addPart(create_turn0_surface, -5, 0.f, 0, 0);
		addPart(create_turn0_surface, -4, 0.f, 1, 1);
		addPart(create_turn0_surface, -3, 0.f, 0, 2);
		addPart(create_turn0_surface, -4, 0.f, -1, 3);
	}

	auto const carCamera1 = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(carCamera1, 0.0f, 8.0f, 110.0f);
		entityRegistry.emplace<vob::aoest::rotation>(carCamera1);
		entityRegistry.emplace<vob::aoest::soft_follow>(
			carCamera1, dynBody, glm::vec3{ 0.0f, 0.0f, -50.0f }, glm::vec3{ 0.0f, 3.0f, 5.0f }, 5.0f, 0.1f);
		entityRegistry.emplace<vob::aoegl::camera_component>(carCamera1);
	}
	auto const carCamera3 = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(carCamera3, 0.0f, 20.0f, 0.0f);
		entityRegistry.emplace<vob::aoest::rotation>(carCamera3);
		entityRegistry.emplace<vob::aoest::attachment_component>(
			carCamera3, dynBody, aoest::combine(glm::vec3{ 0.0f, 1.0f, -1.0f }, glm::quat()));
		entityRegistry.emplace<vob::aoegl::camera_component>(carCamera3);
	}
	auto const carCamera4 = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(carCamera4, -8.0f, 2.0f, 0.0f);
		entityRegistry.emplace<vob::aoest::rotation>(carCamera4);
		entityRegistry.emplace<vob::aoest::attachment_component>(
			carCamera4, dynBody, aoest::combine(glm::vec3{ -8.0f, 0.0f, 0.0f }, glm::angleAxis(-3.141592f / 2, glm::vec3{0.0f, 1.0f, 0.0f})), glm::vec3{1.0f, 0.0f, 1.0f});
		entityRegistry.emplace<vob::aoegl::camera_component>(carCamera4);
	}

	auto& directorWorldComponent = entityRegistry.ctx().get<aoegl::director_world_component>();
	directorWorldComponent.m_activeCamera = carCamera1;

}