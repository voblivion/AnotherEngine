#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/input/bindings.h>
#include <vob/aoe/input/inputs.h>

#include <vob/aoe/spacetime/time_world_component.h>

#include <vob/aoe/engine/world_data_provider.h>


namespace vob::aoein
{
	class VOB_AOE_API binding_system
	{
	public:
		explicit binding_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<bindings> m_bindings;
		aoeng::world_component_ref<inputs> m_inputs;
		aoeng::world_component_ref<
			aoest::presentation_time_world_component const> m_presentationTimeWorldComponent;
	};
}
