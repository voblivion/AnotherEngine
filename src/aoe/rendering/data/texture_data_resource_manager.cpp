#include <vob/aoe/rendering/data/texture_data_resource_manager.h>

#include <vob/misc/std/ignorable_assert.h>


namespace vob::aoegl
{
	graphic_id const& texture_data_resource_manager::add_reference(
		std::shared_ptr<texture_data const> const& a_textureData)
	{
		return m_manager.add_reference(
			a_textureData,
			[](auto const& a_textureData) {
				if (a_textureData == nullptr)
				{
					return graphic_id{};
				}

				constexpr std::array<graphic_enum, 4> k_formats{ GL_RED, GL_RG, GL_RGB, GL_RGBA };
				if (a_textureData->m_channelCount > k_formats.size())
				{
					ignorable_assert(false && "Unsupported texture format.");
					return graphic_id{};
				}

				graphic_id textureId;
				glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
				glBindTexture(GL_TEXTURE_2D, textureId);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTexImage2D(
					GL_TEXTURE_2D
					, 0
					, GL_RGBA
					, a_textureData->m_size.x
					, a_textureData->m_size.y
					, 0
					, k_formats[a_textureData->m_channelCount - 1]
					, GL_UNSIGNED_BYTE
					, a_textureData->m_data.data());

				return textureId;
			});
	}

	void texture_data_resource_manager::remove_reference(
		std::shared_ptr<texture_data const> const& a_textureData)
	{
		return m_manager.remove_reference(
			a_textureData,
			[](auto const& a_textureData, auto const a_textureId)
			{
				glDeleteTextures(1, &a_textureId);
			});
	}
}
