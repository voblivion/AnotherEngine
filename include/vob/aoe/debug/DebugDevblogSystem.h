#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/rendering/DebugMeshContext.h>
#include <vob/aoe/spacetime/Transform.h>


namespace vob::aoedb
{
	class VOB_AOE_API DebugDevblogSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoegl::DebugMeshContext> m_debugMeshContext;
	};
}
