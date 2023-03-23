#include <vob/aoe/rendering/data/texture_file_loader.h>

#include <SFML/Graphics/Image.hpp>

#include <span>


namespace vob::aoegl
{
	texture_data texture_file_loader::load(std::filesystem::path const& a_path) const
	{
		sf::Image image;
		image.loadFromFile(a_path.generic_string());

		auto size = image.getSize();
		auto byteCount = std::size_t{ size.x * size.y * 4ul };
		auto data = std::span{ image.getPixelsPtr(), image.getPixelsPtr() + byteCount };
		
		return {
			4ul
			, glm::uvec2{ size.x, size.y }
			, std::pmr::vector<std::uint8_t>{ data.begin(), data.end() }
		};
	}
}
