#include <vob/aoe/rendering/TextureUtils.h>

#include <vob/aoe/debug/Check.h>

#include <algorithm>
#include <array>
#include <cmath>


namespace vob::aoegl
{
	namespace
	{
		struct ImageFormatInfo
		{
			GraphicEnum linearFormat;
			GraphicEnum srgbFormat;
			GraphicEnum uploadFormat;
		};

		constexpr auto k_imageFormatInfos = std::array{
			ImageFormatInfo{ GL_R8, 0, GL_RED },
			ImageFormatInfo{ GL_RG8, 0, GL_RG },
			ImageFormatInfo{ GL_RGB8, GL_SRGB8, GL_RGB },
			ImageFormatInfo{ GL_RGBA8, GL_SRGB8_ALPHA8, GL_RGBA },
			ImageFormatInfo{ GL_COMPRESSED_RED_RGTC1, 0, 0 },
			ImageFormatInfo{ GL_COMPRESSED_RG_RGTC2, 0, 0 },
			ImageFormatInfo{ GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, 0, 0 },
			ImageFormatInfo{ GL_COMPRESSED_RGBA_BPTC_UNORM, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, 0 }
		};
	}

	GpuTexture createTexture(ImageData const& a_image, TextureSettings const& a_settings)
	{
		auto const invalidTexture = GpuTexture{ k_invalidId, GL_TEXTURE_2D };

		if (!VOB_AOE_CHECK_LOG(
			a_settings.samplerType == TextureSettings::SamplerType::Simple, "Only simple textures are supported for now."))
		{
			return invalidTexture;
		}

		if (!VOB_AOE_CHECK_LOG(!a_image.levels.empty(), "Image has no level."))
		{
			return invalidTexture;
		}

		auto const& baseLevel = a_image.levels.front();
		if (!VOB_AOE_CHECK_LOG(baseLevel.size.x > 0 && baseLevel.size.y > 0, "Empty image."))
		{
			return invalidTexture;
		}

		auto const& formatInfo = k_imageFormatInfos[static_cast<size_t>(a_image.format)];
		auto const useSrgb = a_settings.colorSpace == TextureSettings::ColorSpace::Srgb;
		if (!VOB_AOE_CHECK_LOG(!useSrgb || formatInfo.srgbFormat != 0, "Image format has no srgb variant."))
		{
			return invalidTexture;
		}

		auto const width = static_cast<int32_t>(baseLevel.size.x);
		auto const height = static_cast<int32_t>(baseLevel.size.y);
		auto const internalFormat = useSrgb ? formatInfo.srgbFormat : formatInfo.linearFormat;

		auto const isCompressed = formatInfo.uploadFormat == 0;
		auto const imageLevelCount = static_cast<int32_t>(a_image.levels.size());
		auto const generateMips = !isCompressed && imageLevelCount == 1;
		auto const mipLevels = generateMips
			? static_cast<int32_t>(std::floor(std::log2(std::max(width, height)))) + 1
			: imageLevelCount;

		auto texture = GpuTexture{ k_invalidId, GL_TEXTURE_2D };
		glCreateTextures(texture.target, 1, &texture.id);

		glTextureStorage2D(texture.id, mipLevels, internalFormat, width, height);
		for (auto i = 0; i < imageLevelCount; ++i)
		{
			auto const& level = a_image.levels[i];
			auto const* levelData = a_image.data.data() + level.dataOffset;
			auto const levelWidth = static_cast<int32_t>(level.size.x);
			auto const levelHeight = static_cast<int32_t>(level.size.y);
			if (isCompressed)
			{
				glCompressedTextureSubImage2D(
					texture.id
					, i /* level */
					, 0 /* x offset */
					, 0 /* y offset */
					, levelWidth
					, levelHeight
					, internalFormat
					, static_cast<GraphicSize>(level.dataSize)
					, levelData);
			}
			else
			{
				glTextureSubImage2D(
					texture.id
					, i /* level */
					, 0 /* x offset */
					, 0 /* y offset */
					, levelWidth
					, levelHeight
					, formatInfo.uploadFormat
					, GL_UNSIGNED_BYTE
					, levelData);
			}
		}

		if (generateMips)
		{
			glGenerateTextureMipmap(texture.id);
		}

		glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(texture.id, GL_TEXTURE_SWIZZLE_R, static_cast<GraphicInt>(a_settings.swizzle[0]));
		glTextureParameteri(texture.id, GL_TEXTURE_SWIZZLE_G, static_cast<GraphicInt>(a_settings.swizzle[1]));
		glTextureParameteri(texture.id, GL_TEXTURE_SWIZZLE_B, static_cast<GraphicInt>(a_settings.swizzle[2]));
		glTextureParameteri(texture.id, GL_TEXTURE_SWIZZLE_A, static_cast<GraphicInt>(a_settings.swizzle[3]));

		return texture;
	}
}
