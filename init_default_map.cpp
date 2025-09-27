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
		entityRegistry.emplace<vob::aoest::position>(dynBody, 0.0f, 3.0f, 0.0f);
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

		carCollider.mass = 1'000.0f;
		carCollider.barycenter = glm::vec3{ 0.0f, 0.0f, -0.288295f };
		carCollider.inertia = glm::mat3{ carCollider.mass / 5.0f };
		carCollider.inertia[0][0] *= (0.7f * 0.7f + 1.6f * 1.6f);
		carCollider.inertia[1][1] *= (1.6f * 1.6f + 0.9f * 0.9f);
		carCollider.inertia[2][2] *= (0.9f * 0.9f + 0.7f * 0.7f);

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
			glm::vec3{-5.0f, 2.0f, -5.0f},
			glm::vec3{-200.0f, 20.0f, -5.0f},
			glm::vec3{-5.0f, 2.0f, 200.0f}
		});

		// Looping
		auto const width = 16.0f;
		auto const radius = 16.0f;
		auto const segments = 24;
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

		staticCollider.bounds = vob::aoeph::aabb{ glm::vec3{-200.0f, 2.0f, -5.0f}, glm::vec3{200.0f, 34.0f, 200.0f} };
	}

	auto const carCamera1 = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(carCamera1, 0.0f, 20.0f, 0.0f);
		entityRegistry.emplace<vob::aoest::rotation>(carCamera1);
		entityRegistry.emplace<vob::aoest::soft_follow>(
			carCamera1, dynBody, glm::vec3{ 0.0f, 0.0f, -5.0f }, glm::vec3{ 0.0f, 5.0f, 10.0f }, 5.0f, 0.99f);
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
	/* auto const carCamera4 = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(carCamera4, 0.0f, 20.0f, 0.0f);
		entityRegistry.emplace<vob::aoest::rotation>(carCamera4);
		entityRegistry.emplace<vob::aoest::soft_follow>(
			carCamera4, dynBody, glm::vec3{ 0.0f, 0.0f, -2.0f }, glm::vec3{ 0.0f, 1.0f, 2.0f }, 0.0f);
		entityRegistry.emplace<vob::aoegl::camera_component>(carCamera4);
	}*/

	auto& directorWorldComponent = entityRegistry.ctx().get<aoegl::director_world_component>();
	directorWorldComponent.m_activeCamera = carCamera1;

}