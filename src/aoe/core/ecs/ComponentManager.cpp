#include <vob/aoe/core/ecs/ComponentManager.h>


namespace vob::aoe::ecs
{
	// Public
	std::pmr::polymorphic_allocator<std::byte> ComponentManager::getAllocator() const
	{
		return m_components.get_allocator();
	}
}