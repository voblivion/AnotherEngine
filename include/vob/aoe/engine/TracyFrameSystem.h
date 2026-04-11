#pragma once

#include <vob/aoe/api.h>

#include <tracy/Tracy.hpp>


namespace vob::aoeng
{
	class EcsWorldDataAccessProvider;
	class EcsWorldDataAccessRegistrar;

	class TracyFrameSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar) {}

		void execute([[maybe_unused]] aoeng::EcsWorldDataAccessProvider const& a_wdp) const { FrameMark; };
	};
}
