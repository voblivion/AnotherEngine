#pragma once

#include <cwchar>
#include <fstream>
#include <vector>

#include <vob/aoe/common/data/filesystem/AFileSystemLoader.h>
#include <vob/aoe/common/render/Manager.h>
#include <vob/aoe/common/render/resources/Texture.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>

namespace vob::aoe::common
{
	class TextureLoader final
		: public AFileSystemLoader
	{
	public:
		// Constructors
		explicit TextureLoader(IGraphicResourceManager<Texture>& a_textureManager)
			: m_textureManager{ a_textureManager }
		{}

		// Methods
		bool canLoad(std::filesystem::path const& a_path) const override
		{
			constexpr std::array<wchar_t const*, 8> supportedExtensions{
				L".bmp"
				, L".png"
				, L".tga"
				, L".jpg"
				, L".gif"
				, L".psd"
				, L".hdr"
				, L".pic"
			};

			auto extension = a_path.extension();
			auto const supportedExtensionIt = std::find_if(
				supportedExtensions.begin()
				, supportedExtensions.end()
				, [&extension] (auto a_supportedExtension) {
					return std::wcscmp(a_supportedExtension, extension.c_str()) == 0;
				}
			);

			return supportedExtensionIt != supportedExtensions.end();
		}

		std::shared_ptr<ADynamicType> load(std::filesystem::path const& a_path) const override
		{
			sf::Image image;
			auto result = loadImage(a_path, image);
			if (!loadImage(a_path, image))
			{
				return nullptr;
			}

			// TODO should probably load an Image then change the way Texture is generated
			return std::make_shared<GraphicResourceHandle<Texture>>(
				m_textureManager
				, std::move(image)
			);
		}

	private:
		// Attributes
		IGraphicResourceManager<Texture>& m_textureManager;

		// Methods
		bool loadImage(std::filesystem::path const& a_path, sf::Image& a_image) const
		{
			return a_image.loadFromFile(a_path.generic_string());
		}
	};
}
