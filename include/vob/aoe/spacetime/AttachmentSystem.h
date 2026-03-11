#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/spacetime/Transform.h>
#include <vob/aoe/spacetime/AttachmentComponent.h>


namespace vob::aoest
{
	class VOB_AOE_API AttachmentSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const> m_transformEntities;
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, AttachmentComponent> m_attachedEntities;
	};
}
