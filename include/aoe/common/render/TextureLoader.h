#pragma once

#include <vector>

#include <aoe/core/data/ALoader.h>
#include <aoe/common/render/Texture.h>
#include <aoe/core/standard/Allocator.h>
#include <aoe/core/standard/Memory.h>
#include <aoe/common/render/SfmlInputStream.h>

namespace aoe
{
	namespace common
	{
		class TextureLoader final
			: public data::ALoader
		{
		public:
			// Constructors
			explicit TextureLoader(std::pmr::memory_resource* a_resource
				= std::pmr::get_default_resource())
				: m_resource{ a_resource }
			{}

			// Methods
			virtual std::shared_ptr<ADynamicType> load(
				std::istream& a_inputStream) override
			{
				// Get stream size
				auto const t_startPos = a_inputStream.tellg();
				a_inputStream.seekg(0, std::ios::end);
				auto const t_endPos = a_inputStream.tellg();
				a_inputStream.seekg(t_startPos, std::ios::beg);

				// Copy stream into vector
				std::pmr::vector<char> t_source;
				t_source.resize(t_endPos - t_startPos);
				ignorableAssert(!t_source.empty());
				if(t_source.empty())
				{
					return nullptr;
				}
				a_inputStream.read(&t_source[0], t_source.size());

				// Load SFML texture
				sf::Texture t_texture;
				t_texture.loadFromMemory(&t_source[0], t_source.size());

				return sta::allocatePolymorphicWith<Texture>(m_resource
					, std::move(t_texture));
			}

		private:
			// Attributes
			std::pmr::memory_resource* m_resource;
		};
	}
}
