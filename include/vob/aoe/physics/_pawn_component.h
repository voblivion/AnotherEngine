#pragma once

#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>

#include <memory>


namespace vob::aoeph
{
	struct pawn_component
	{
		std::shared_ptr<btCapsuleShapeZ> m_shape = std::make_shared<btCapsuleShapeZ>(btScalar(0.5), btScalar(1.0));
	};
}
