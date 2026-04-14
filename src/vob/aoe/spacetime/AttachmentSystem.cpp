#include <vob/aoe/spacetime/AttachmentSystem.h>

#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"
#include "vob/aoe/spacetime/TransformUtils.h"

#include <glm/gtc/quaternion.hpp>


namespace vob::aoest
{
	void AttachmentSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void AttachmentSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		for (auto [entity, positionCmp, rotationCmp, attachmentCmp] : m_attachedEntities.get(a_wdap).each())
		{
			if (!m_transformEntities.get(a_wdap).contains(attachmentCmp.target))
			{
				continue;
			}

			auto const& [attachedPositionCmp, attachedRotationCmp] = m_transformEntities.get(a_wdap).get(attachmentCmp.target);
			positionCmp.value = transformPosition(attachedPositionCmp, attachedRotationCmp, attachmentCmp.offsetPosition);
			rotationCmp.value = attachedRotationCmp.value * attachmentCmp.offsetRotation;
		}
	}
}
