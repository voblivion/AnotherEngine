#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/window/WindowContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>


namespace vob::aoewi
{
	class VOB_AOE_API SwapBuffersSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdp) const;

	private:
		aoeng::EcsWorldContextRef<WindowContext> m_windowContext;
	};
}
