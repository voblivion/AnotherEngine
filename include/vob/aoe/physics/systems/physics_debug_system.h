#pragma once

#include <vob/aoe/physics/debug_drawer.h>
#include <vob/aoe/physics/world_components/physics_debug_world_component.h>
#include <vob/aoe/physics/world_components/physics_world_component.h>

#include <vob/aoe/api.h>
#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/input/bindings.h>


namespace vob::aoeph
{
	class physics_debug_system
	{
	public:
		explicit physics_debug_system(aoeng::world_data_provider& a_wdp)
			: m_physicsWorldComponent{ a_wdp }
			, m_physicsDebugWorldComponent{ a_wdp }
			, m_debugMeshWorldComponent{ a_wdp }
			, m_bindings{ a_wdp }
		{
		}

		void update() const
		{
			if (auto const cycleDebugDrawModeSwitch = m_bindings->switches.find(
				m_physicsDebugWorldComponent->m_cycleDebugDrawModeBinding))
			{
				if (cycleDebugDrawModeSwitch->was_pressed())
				{
					m_physicsDebugWorldComponent->m_debugDrawModeIndex =
						(m_physicsDebugWorldComponent->m_debugDrawModeIndex + 1) % k_debugDrawModes.size();
				}
			}

			if (m_physicsDebugWorldComponent->m_debugDrawModeIndex != 0)
			{
				debug_drawer debugDrawer{ *m_debugMeshWorldComponent };
				debugDrawer.setDebugMode(m_physicsDebugWorldComponent->m_debugDrawModeIndex);
				auto& physicsWorld = m_physicsWorldComponent->m_world.get();
				physicsWorld.setDebugDrawer(&debugDrawer);
				physicsWorld.debugDrawWorld();
				physicsWorld.setDebugDrawer(nullptr);
			}

			auto const& cycleDebugDrawModeSwitch = m_bindings->switches;
		}

	private:
		aoeng::world_component_ref<physics_world_component> m_physicsWorldComponent;
		aoeng::world_component_ref<physics_debug_world_component> m_physicsDebugWorldComponent;
		aoeng::world_component_ref<aoegl::debug_mesh_world_component> m_debugMeshWorldComponent;
		aoeng::world_component_ref<aoein::bindings const> m_bindings;
	};
}
