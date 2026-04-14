#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"
#include <vob/aoe/spacetime/AttachmentComponent.h>


namespace vob::aoest
{
	class VOB_AOE_API AttachmentSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldViewRef<aoest::PositionComponent const, aoest::RotationComponent const> m_transformEntities;
		aoeng::EcsWorldViewRef<aoest::PositionComponent, aoest::RotationComponent, AttachmentComponent> m_attachedEntities;
	};
}
