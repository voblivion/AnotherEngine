#pragma once

#include <LinearMath/btScalar.h>
#include <vob/misc/visitor/macros.h>


namespace vob::aoeph
{
	struct material
	{
		btScalar m_restitution{ 0.2f };
		btScalar m_friction{ 0.5f };
		btScalar m_rollingFriction{ 0.01f };
		btScalar m_spinningFriction{ 0.1f };
		btScalar m_contactStiffness{ 9.99999984e+17f };
		btScalar m_contactDamping{ 0.1f };
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoeph::material)
	{
		VOB_MISVI_NVP("Restitution", restitution);
		VOB_MISVI_NVP("Friction", friction);
		VOB_MISVI_NVP("Rolling Friction", rollingFriction);
		VOB_MISVI_NVP("Spinning Friction", spinningFriction);
		VOB_MISVI_NVP("Contact Stiffness", contactStiffness);
		VOB_MISVI_NVP("Contact Damping", contactDamping);
		return true;
	}
}
