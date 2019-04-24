#include <aoe/core/ecs/WorldData.h>


namespace aoe
{
	namespace ecs
	{
		WorldData::WorldData(sta::Allocator<std::byte> const& a_allocator)
			: m_worldComponents{ a_allocator }
			, m_entityManager{ a_allocator }
		{}

		void WorldData::update()
		{
			m_entityManager.update();
		}
	}
}