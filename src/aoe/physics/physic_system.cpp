#include <vob/aoe/physics/physic_system.h>


namespace vob::aoeph
{
	physic_system::physic_system(aoeng::world_data_provider& a_wdp)
		: m_physicsWorldComponent{ a_wdp }
		, m_simulationTimeWorldComponent{ a_wdp }
		, m_rigidbodyEntities{ a_wdp }
	{}

	void physic_system::update() const
	{
		auto& physicsWorld = m_physicsWorldComponent->m_world.get();

		physicsWorld.stepSimulation(m_simulationTimeWorldComponent->m_elapsedTime.get_value());

		auto rigidbodyEntityView = m_rigidbodyEntities.get();
		for (auto rigidbodyEntity : rigidbodyEntityView)
		{
			auto [transform, rigidbody] = rigidbodyEntityView.get(rigidbodyEntity);

#pragma message(VOB_MISTD_TODO "apply physics output to transforms.")
		}
	}
}
