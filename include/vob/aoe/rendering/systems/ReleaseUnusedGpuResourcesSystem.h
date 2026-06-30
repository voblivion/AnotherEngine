#pragma once

#include "vob/aoe/rendering/contexts/GpuResourceRegistriesContext.h"

#include "vob/aoe/engine/EcsWorldDataAccess.h"


namespace vob::aoegl
{
	class ReleaseUnusedGpuResourcesSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<GpuResourceRegistriesContext> m_gpuResourceRegistriesContext;
	};
}
