#pragma once

#include <vob/aoe/spacetime/attachment_component.h>
#include <vob/aoe/spacetime/transform.h>

#include <vob/aoe/engine/world_data_provider.h>


namespace vob::aoest
{
	namespace
	{
		template <typename TTransformEntities, typename TAttachmentEntities>
		void update_attachment(
			position& a_position,
			rotation& a_rotation,
			attachment_component& a_attachment,
			TAttachmentEntities const& a_attachmentEntities,
			TTransformEntities const& a_transformEntities)
		{
			if (!a_attachment.m_needsUpdate)
			{
				return;
			}
			a_attachment.m_needsUpdate = false;

			glm::mat4 parentTransform{ 1.0f };
			if (a_attachmentEntities.contains(a_attachment.m_parent))
			{
				auto [parentPosition, parentRotation, parentAttachment] =
					a_attachmentEntities.get(a_attachment.m_parent);
				update_attachment(
					parentPosition,
					parentRotation,
					parentAttachment,
					a_attachmentEntities,
					a_transformEntities);
				parentTransform = aoest::combine(parentPosition, parentRotation);
			}
			else if (a_transformEntities.contains(a_attachment.m_parent))
			{
				auto [parentPosition, parentRotation] = a_transformEntities.get(a_attachment.m_parent);
				parentTransform = aoest::combine(parentPosition, parentRotation);
			}

			auto const transform = parentTransform * a_attachment.m_localTransform;
			a_position = transform[3];
			a_rotation = glm::quat_cast(transform);
		}
	}

	class attachment_system
	{
	public:
		explicit attachment_system(aoeng::world_data_provider& a_wdp)
			: m_transformEntities{ a_wdp }
			, m_attachmentEntities{ a_wdp }
		{}

		void update() const
		{
			auto attachmentEntities = m_attachmentEntities.get();
			for (auto const attachmentEntity : attachmentEntities)
			{
				auto& attachment = attachmentEntities.get<attachment_component>(attachmentEntity);
				if (attachment.m_parent != entt::tombstone)
				{
					attachment.m_needsUpdate = true;
				}
			}

			auto transformEntities = m_transformEntities.get();
			for (auto const attachmentEntity : attachmentEntities)
			{
				auto [position, rotation, attachment] = attachmentEntities.get(attachmentEntity);
				update_attachment(position, rotation, attachment, attachmentEntities, transformEntities);				
			}
		}

	private:
		aoeng::registry_view_ref<position const, rotation const> m_transformEntities;
		aoeng::registry_view_ref<position, rotation, attachment_component> m_attachmentEntities;
	};
}
