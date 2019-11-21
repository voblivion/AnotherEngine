#include <bullet/btBulletDynamicsCommon.h>

int test()
{
	btDefaultCollisionConfiguration* collisionConfig = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfig);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(
		dispatcher, overlappingPairCache, solver, collisionConfig);

	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	{
		btCollisionShape* groundShape = new btBoxShape(btVector3(
			btScalar(50.), btScalar(50.), btScalar(50.)));
		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -56, 0));

		btScalar mass(0.);

		bool isDynamic = (mass != 0.);

		btVector3 localIntertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localIntertia);

		btDefaultMotionState* motionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(
			mass, motionState, groundShape, localIntertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
	}

	{
		btCollisionShape* sphereShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(sphereShape);

		btTransform sphereTransform;
		sphereTransform.setIdentity();

		btScalar mass(1.);

		bool isDynamic = (mass != 0.);

		btVector3 localIntertia(0, 0, 0);
		if (isDynamic)
			sphereShape->calculateLocalInertia(mass, localIntertia);

		sphereTransform.setOrigin(btVector3(2, 10, 0));

		btDefaultMotionState* motionState = new btDefaultMotionState(sphereTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(
			mass, motionState, sphereShape, localIntertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
	}

	for(int i = 0; i < 150; i++)
	{
		dynamicsWorld->stepSimulation(1.f / 60.f, 1);

		for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
		{
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
			btRigidBody* body = btRigidBody::upcast(obj);
			btTransform trans;
			if (body && body->getMotionState())
			{
				body->getMotionState()->getWorldTransform(trans);
			}
			else
			{
				trans = obj->getWorldTransform();
			}
			printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
		}
	}

	return EXIT_SUCCESS;
}