#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/spacetime/presentation_time_world_component.h>

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/world_component_ref.h>


namespace vob::aoest
{
	class presentation_time_system
	{
	public:
		explicit presentation_time_system(aoecs::world_data_provider& a_wdp)
			: m_presentationTimeWorldComponent{ a_wdp }
		{}

		void update() const
		{
			auto const currentTime = presentation_time_world_component::clock::now();
			if (m_presentationTimeWorldComponent->m_frameStartTime
				!= presentation_time_world_component::time_point{})
			{
				m_presentationTimeWorldComponent->m_frameDuration =
					currentTime - m_presentationTimeWorldComponent->m_frameStartTime;
				m_presentationTimeWorldComponent->m_elapsedTime =
					m_presentationTimeWorldComponent->m_frameDuration;
			}
			m_presentationTimeWorldComponent->m_frameStartTime = currentTime;
		}

	private:
		// TODO pause?
		aoecs::world_component_ref<presentation_time_world_component>
			m_presentationTimeWorldComponent;
	};
}
