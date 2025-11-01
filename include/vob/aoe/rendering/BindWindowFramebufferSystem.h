#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/window/WindowContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>


namespace vob::aoegl
{
	class VOB_AOE_API BindWindowFramebufferSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoewi::WindowContext> m_windowContext;
	};
}
