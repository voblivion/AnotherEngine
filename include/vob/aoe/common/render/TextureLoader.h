#pragma once

#include <vector>

#include <vob/aoe/core/data/ALoader.h>
#include <vob/sta/memory.h>
#include <vob/aoe/common/render/SfmlInputStream.h>
#include <vob/aoe/common/opengl/Manager.h>
#include <vob/aoe/common/opengl/resources/Texture.h>
#include <vob/aoe/common/opengl/Handle.h>

namespace vob::aoe::ogl
{
	class TextureLoader final
		: public data::ALoader
	{
	public:
		// Constructors
		explicit TextureLoader(
			Manager<Texture>& a_textureManager
			, std::pmr::memory_resource* a_memoryResource = std::pmr::get_default_resource()
		)
			: m_textureManager{ a_textureManager }
			, m_memoryResource{ a_memoryResource }
		{}

		// Methods
		std::shared_ptr<ADynamicType> load(std::istream& a_inputStream) override
		{
			// Get stream size
			auto const t_startPos = a_inputStream.tellg();
			a_inputStream.seekg(0, std::ios::end);
			auto const t_endPos = a_inputStream.tellg();
			a_inputStream.seekg(t_startPos, std::ios::beg);

			// Copy stream into vector
			std::pmr::vector<char> t_source;
			t_source.resize(t_endPos - t_startPos);
			ignorable_assert(!t_source.empty());
			if(t_source.empty())
			{
				return nullptr;
			}
			a_inputStream.read(&t_source[0], t_source.size());

			// Load SFML texture
			sf::Image t_image;
			t_image.loadFromMemory(&t_source[0], t_source.size());

			return std::allocate_shared<Handle<Texture>>(
				std::pmr::polymorphic_allocator<Handle<Texture>>{ m_memoryResource }
				, m_textureManager
				, m_memoryResource
				, std::move(t_image)
			);
		}

	private:
		// Attributes
		Manager<Texture>& m_textureManager;
		std::pmr::memory_resource* m_memoryResource;
	};
}
