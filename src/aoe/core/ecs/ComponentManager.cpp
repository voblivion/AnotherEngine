#include <aoe/core/ecs/ComponentManager.h>


namespace aoe
{
	namespace ecs
	{
		// Public
		sta::Allocator<std::byte> ComponentManager::getAllocator() const
		{
			return m_components.get_allocator();
		}
	}
}