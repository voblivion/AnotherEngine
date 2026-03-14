#include <vob/aoe/spacetime/AttachmentSystem.h>

#include <vob/aoe/spacetime/Transform.h>

#include <glm/gtc/quaternion.hpp>


namespace vob::aoest
{
	void AttachmentSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void AttachmentSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		for (auto [entity, position, rotation, attachmentCmp] : m_attachedEntities.get(a_wdap).each())
		{
			if (!m_transformEntities.get(a_wdap).contains(attachmentCmp.target))
			{
				continue;
			}

			auto const& [attachedPosition, attachedRotation] = m_transformEntities.get(a_wdap).get(attachmentCmp.target);
			position = attachedPosition + attachedRotation * attachmentCmp.offsetPosition;
			rotation = attachedRotation * attachmentCmp.offsetRotation;
		}
	}
}
