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
#include <vob/aoe/physics/components/car_controller.h>
#include <vob/aoe/physics/world_components/physics_world_component.h>
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

	auto const track = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(track, 0.0f, 0.0f, 0.0f);
		entityRegistry.emplace<vob::aoest::rotation>(track);

		auto trackModelId = a_data.filesystemIndexer.get_runtime_id("data/new/models/Track.gltf");
		entityRegistry.emplace<vob::aoegl::model_data_component>(
			track,
			a_data.modelDatabase.find(trackModelId));

		// aya .. mem leak and all, disgusting!
		auto model = a_data.modelDatabase.find(trackModelId);
		auto& mesh = model->m_texturedMeshes[0].m_mesh;
		btTriangleIndexVertexArray* vertexArray = new btTriangleIndexVertexArray();
		btIndexedMesh indexedMesh;
		indexedMesh.m_numTriangles = static_cast<int>(mesh.m_triangles.size());
		indexedMesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(&(mesh.m_triangles[0].m_v0));
		indexedMesh.m_triangleIndexStride = sizeof(vob::aoegl::triangle);
		indexedMesh.m_numVertices = static_cast<int>(mesh.m_positions.size());
		indexedMesh.m_vertexBase = reinterpret_cast<const unsigned char*>(&(mesh.m_positions[0].x));
		indexedMesh.m_vertexStride = sizeof(glm::vec3);
		vertexArray->addIndexedMesh(indexedMesh);
		entityRegistry.emplace<vob::aoeph::rigidbody>(
			track,
			true,
			0.0f,
			glm::mat4{1.0f},
			std::make_shared<btBvhTriangleMeshShape>(vertexArray, true),
			std::make_shared<aoeph::material>());
	}

	auto const ghostControlledCamera = entityRegistry.create();
	{

		entityRegistry.emplace<vob::aoest::position>(ghostControlledCamera, 0.0f, 10.0f, 30.0f);
		entityRegistry.emplace<vob::aoest::rotation>(ghostControlledCamera);

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
		/*ghostController.m_decreaseSpeedMapping = bindings.switches.add(aoein::binding_util::make_switch(
			aoein::mouse::button::ScrollDown));
		ghostController.m_increaseSpeedMapping = bindings.switches.add(aoein::binding_util::make_switch(
			aoein::mouse::button::ScrollUp));*/

		entityRegistry.emplace<vob::aoegl::camera_component>(ghostControlledCamera);

		entityRegistry.emplace<vob::aoedb::controllable_tag>(ghostControlledCamera);
		entityRegistry.emplace<vob::aoedb::controlled_tag>(ghostControlledCamera);
	}

	auto const car = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(car, 0.0f, 20.0f, 0.0f);
		entityRegistry.emplace<vob::aoest::rotation>(car);
		auto& carController = entityRegistry.emplace<vob::aoeph::car_controller>(car);
		entityRegistry.emplace<vob::aoedb::controllable_tag>(car);

		auto& ghostController = entityRegistry.emplace<vob::aoedb::debug_ghost_controller_component>(car);
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
		/*ghostController.m_decreaseSpeedMapping = bindings.switches.add(aoein::binding_util::make_switch(
			aoein::mouse::button::ScrollDown));
		ghostController.m_increaseSpeedMapping = bindings.switches.add(aoein::binding_util::make_switch(
			aoein::mouse::button::ScrollUp));*/


		entityRegistry.emplace<aoeph::rigidbody>(
			car,
			false,
			1000.0f /* mass */,
			glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, -10.0f, 0.0f }),
			std::make_shared<btBoxShape>(btVector3{ 0.75f, 0.20f, 1.2f }),
			std::make_shared<aoeph::material>(0.5f, 1.0f, 0.01f, 0.2f));


		/*auto& physicsWorldCmp = a_world.get_world_component<vob::aoeph::physics_world_component>();

		btCollisionShape* chassisShape = new btBoxShape(btVector3(0.75f, 0.25f, 1.2f));
		carController.m_collisionShapes.emplace_back(chassisShape);
		
		btCompoundShape* compound = new btCompoundShape();
		carController.m_collisionShapes.emplace_back(compound);
		btTransform localTrans;
		localTrans.setIdentity();
		localTrans.setOrigin(btVector3(0, 1, 0));

		compound->addChildShape(localTrans, chassisShape);
		{
			btCollisionShape* suppShape = new btBoxShape(btVector3(0.5f, 0.1f, 0.5f));
			carController.m_collisionShapes.emplace_back(suppShape);

			btTransform suppLocalTrans;
			suppLocalTrans.setIdentity();
			suppLocalTrans.setOrigin(btVector3(0, 1.0, 2.5));
			compound->addChildShape(suppLocalTrans, suppShape);
		}
		
		btTransform tr;
		tr.setIdentity();
		const btScalar fallHeight = 5.0f;
		tr.setOrigin(btVector3(0, fallHeight, 0));
		
		btScalar chassisMass = 2.0f;
		btVector3 localInertia(0, 0, 0);
		compound->calculateLocalInertia(chassisMass, localInertia);
		btDefaultMotionState* motionState = new btDefaultMotionState(tr);
		btRigidBody::btRigidBodyConstructionInfo chassisConstructInfo(chassisMass, motionState, compound, localInertia);
		btRigidBody* chassisBody = new btRigidBody(chassisConstructInfo);
		physicsWorldCmp.m_world.get().addRigidBody(chassisBody);
		carController.m_chassisRigidBody.reset(chassisBody);

		btScalar wheelWidth = 0.5f;
		btScalar wheelRadius = 0.37f;
		btCylinderShapeX* wheelShape = new btCylinderShapeX(btVector3(wheelWidth, wheelRadius, wheelRadius));
		carController.m_collisionShapes.emplace_back(wheelShape);

		btScalar wheelMass = 1.0f;
		btVector3 wheelPos[4] = {
			btVector3(-0.75, fallHeight - 0.125, 0.7),
			btVector3(0.75, fallHeight - 0.125, 0.7),
			btVector3(0.75, fallHeight - 0.125, -0.7),
			btVector3(-0.75, fallHeight - 0.125, -0.7)
		};

		chassisBody->setActivationState(DISABLE_DEACTIVATION);
		for (int i = 0; i < 4; ++i)
		{
			btTransform wTr;
			wTr.setIdentity();
			wTr.setOrigin(wheelPos[i]);

			btVector3 wLocalInertia(0, 0, 0);
			wheelShape->calculateLocalInertia(wheelMass, wLocalInertia);
			btDefaultMotionState* wMotionState = new btDefaultMotionState(wTr);
			btRigidBody::btRigidBodyConstructionInfo wConstructInfo(wheelMass, wMotionState, wheelShape, wLocalInertia);
			btRigidBody* wheelBody = new btRigidBody(wConstructInfo);
			carController.m_wheelRigidBodies.emplace_back(wheelBody, vob::aoeph::to_glm(wheelPos[i]));
			wheelBody->setUserIndex(-1);
			physicsWorldCmp.m_world.get().addRigidBody(wheelBody);
			wheelBody->setFriction(1110);
			wheelBody->setActivationState(DISABLE_DEACTIVATION);

			btVector3 parentAxis(0, 1, 0);
			btVector3 childAxis(1, 0, 0);
			btVector3 anchor = tr.getOrigin();
			btHinge2Constraint* pHinge2 = new btHinge2Constraint(*chassisBody, *wheelBody, anchor, parentAxis, childAxis);
			physicsWorldCmp.m_world.get().addConstraint(pHinge2, true);
			carController.m_wheelHinges.emplace_back(pHinge2);

			// drive engine
			pHinge2->enableMotor(3, true);
			pHinge2->setMaxMotorForce(3, 1000);
			pHinge2->setTargetVelocity(3, 0);

			// steer engine
			pHinge2->enableMotor(5, true);
			pHinge2->setMaxMotorForce(5, 1000);
			pHinge2->setTargetVelocity(5, 0);
			pHinge2->setParam(BT_CONSTRAINT_CFM, 0.15f, 2);
			pHinge2->setParam(BT_CONSTRAINT_ERP, 0.35f, 2);

			pHinge2->setDamping(2, 2.0);
			pHinge2->setStiffness(2, 40.0f);

			pHinge2->setDbgDrawSize(btScalar(5.f));
		}*/
	}
	auto const carCamera1 = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(carCamera1, 0.0f, 20.0f, 0.0f);
		entityRegistry.emplace<vob::aoest::rotation>(carCamera1);
		entityRegistry.emplace<vob::aoest::soft_follow>(
			carCamera1, car, glm::vec3{ 0.0f, 0.0f, -5.0f }, glm::vec3{ 0.0f, 5.0f, 10.0f });
		entityRegistry.emplace<vob::aoegl::camera_component>(carCamera1);
	}
	auto const carCamera3 = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(carCamera3, 0.0f, 20.0f, 0.0f);
		entityRegistry.emplace<vob::aoest::rotation>(carCamera3);
		entityRegistry.emplace<vob::aoest::soft_follow>(
			carCamera3, car, glm::vec3{ 0.0f, 0.0f, -2.0f }, glm::vec3{ 0.0f, 0.5f, 00.0f }, 0.0f);
		entityRegistry.emplace<vob::aoegl::camera_component>(carCamera3);
	}
	auto const carCamera4 = entityRegistry.create();
	{
		entityRegistry.emplace<vob::aoest::position>(carCamera4, 0.0f, 20.0f, 0.0f);
		entityRegistry.emplace<vob::aoest::rotation>(carCamera4);
		entityRegistry.emplace<vob::aoest::soft_follow>(
			carCamera4, car, glm::vec3{ 0.0f, 0.0f, -2.0f }, glm::vec3{ 0.0f, 1.0f, 2.0f }, 0.0f);
		entityRegistry.emplace<vob::aoegl::camera_component>(carCamera4);
	}

	auto& directorWorldComponent = entityRegistry.ctx().get<aoegl::director_world_component>();
	directorWorldComponent.m_activeCamera = ghostControlledCamera;
}