#include "vob/aoe/rendering/data/ImageLoader.h"

#include "vob/aoe/debug/Check.h"

#include "stb_image.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <vector>


namespace vob::aoegl
{
	namespace
	{
		constexpr auto k_ktx2Identifier = std::array<uint8_t, 12>{
			0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

		bool isKtx2(std::istream& a_stream)
		{
			auto identifier = decltype(k_ktx2Identifier){};
			a_stream.read(reinterpret_cast<char*>(identifier.data()), identifier.size());
			auto const readCount = a_stream.gcount();

			a_stream.clear();
			a_stream.seekg(0);

			return readCount == static_cast<std::streamsize>(identifier.size())
				&& identifier == k_ktx2Identifier;
		}

		struct Ktx2Format
		{
			uint32_t vkFormat;
			ImageData::Format format;
			TextureSettings::ColorSpace colorSpace;
			uint32_t unitByteSize;
			bool isBlockCompressed;
		};

		constexpr auto k_ktx2Formats = std::array{
			Ktx2Format{ 9, ImageData::Format::R8, TextureSettings::ColorSpace::Linear, 1, false },
			Ktx2Format{ 16, ImageData::Format::Rg8, TextureSettings::ColorSpace::Linear, 2, false },
			Ktx2Format{ 23, ImageData::Format::Rgb8, TextureSettings::ColorSpace::Linear, 3, false },
			Ktx2Format{ 29, ImageData::Format::Rgb8, TextureSettings::ColorSpace::Srgb, 3, false },
			Ktx2Format{ 37, ImageData::Format::Rgba8, TextureSettings::ColorSpace::Linear, 4, false },
			Ktx2Format{ 43, ImageData::Format::Rgba8, TextureSettings::ColorSpace::Srgb, 4, false },
			Ktx2Format{ 131, ImageData::Format::Bc1, TextureSettings::ColorSpace::Linear, 8, true },
			Ktx2Format{ 132, ImageData::Format::Bc1, TextureSettings::ColorSpace::Srgb, 8, true },
			Ktx2Format{ 133, ImageData::Format::Bc1a, TextureSettings::ColorSpace::Linear, 8, true },
			Ktx2Format{ 134, ImageData::Format::Bc1a, TextureSettings::ColorSpace::Srgb, 8, true },
			Ktx2Format{ 139, ImageData::Format::Bc4, TextureSettings::ColorSpace::Linear, 8, true },
			Ktx2Format{ 141, ImageData::Format::Bc5, TextureSettings::ColorSpace::Linear, 16, true },
			Ktx2Format{ 143, ImageData::Format::Bc6h, TextureSettings::ColorSpace::Linear, 16, true },
			Ktx2Format{ 145, ImageData::Format::Bc7, TextureSettings::ColorSpace::Linear, 16, true },
			Ktx2Format{ 146, ImageData::Format::Bc7, TextureSettings::ColorSpace::Srgb, 16, true } };

		size_t computeKtx2LevelByteSize(Ktx2Format const& a_format, glm::uvec2 const& a_size)
		{
			if (a_format.isBlockCompressed)
			{
				auto const blockCount = ((a_size.x + 3) / 4) * ((a_size.y + 3) / 4);
				return size_t{ blockCount } * a_format.unitByteSize;
			}
			return size_t{ a_size.x } * a_size.y * a_format.unitByteSize;
		}

		ImageData loadKtx2Image(std::istream& a_stream)
		{
			struct Header
			{
				uint32_t vkFormat;
				uint32_t typeSize;
				uint32_t pixelWidth;
				uint32_t pixelHeight;
				uint32_t pixelDepth;
				uint32_t layerCount;
				uint32_t faceCount;
				uint32_t levelCount;
				uint32_t supercompressionScheme;
			};
			static constexpr auto k_indexByteSize = std::streamoff{ 32 };

			a_stream.seekg(static_cast<std::streamoff>(k_ktx2Identifier.size()));

			auto header = Header{};
			a_stream.read(reinterpret_cast<char*>(&header), sizeof(header));
			if (!VOB_AOE_CHECK_LOG(
				a_stream.gcount() == static_cast<std::streamsize>(sizeof(header)), "Truncated ktx2 header."))
			{
				return ImageData{};
			}

			if (!VOB_AOE_CHECK_LOG(
				header.supercompressionScheme == 0
				, "Unsupported ktx2 supercompression scheme ({}); export without supercompression."
				, header.supercompressionScheme))
			{
				return ImageData{};
			}

			if (!VOB_AOE_CHECK_LOG(
				header.pixelDepth == 0 && header.layerCount == 0 && header.faceCount == 1
				, "Only 2d non-array ktx2 images are supported."))
			{
				return ImageData{};
			}

			auto const formatIt = std::ranges::find(k_ktx2Formats, header.vkFormat, &Ktx2Format::vkFormat);
			if (!VOB_AOE_CHECK_LOG(
				formatIt != k_ktx2Formats.end(), "Unsupported ktx2 vulkan format ({}).", header.vkFormat))
			{
				return ImageData{};
			}

			struct LevelIndexEntry
			{
				uint64_t byteOffset;
				uint64_t byteLength;
				uint64_t uncompressedByteLength;
			};

			auto const levelCount = std::max(static_cast<int32_t>(header.levelCount), 1);
			auto const levelIndexByteSize =
				static_cast<std::streamsize>(levelCount * sizeof(LevelIndexEntry));

			auto levelIndex = std::vector<LevelIndexEntry>(static_cast<size_t>(levelCount));
			a_stream.seekg(k_indexByteSize, std::ios::cur);
			a_stream.read(reinterpret_cast<char*>(levelIndex.data()), levelIndexByteSize);
			if (!VOB_AOE_CHECK_LOG(
				a_stream.gcount() == levelIndexByteSize, "Truncated ktx2 level index."))
			{
				return ImageData{};
			}

			auto image = ImageData{
				.format = formatIt->format, .colorSpace = formatIt->colorSpace };

			auto dataOffset = size_t{ 0 };
			for (auto levelIt = int32_t{ 0 }; levelIt < levelCount; ++levelIt)
			{
				auto const levelSize = glm::uvec2{
					std::max(header.pixelWidth >> levelIt, 1u), std::max(header.pixelHeight >> levelIt, 1u) };
				auto const expectedByteSize = computeKtx2LevelByteSize(*formatIt, levelSize);
				if (!VOB_AOE_CHECK_LOG(
					levelIndex[levelIt].byteLength == expectedByteSize
					, "Ktx2 level {} is {} bytes, expected {} for its format and size."
					, levelIt, levelIndex[levelIt].byteLength, expectedByteSize))
				{
					return ImageData{};
				}

				image.levels.emplace_back(levelSize, dataOffset, expectedByteSize);
				dataOffset += expectedByteSize;
			}

			image.data.resize(dataOffset);
			for (auto levelIt = int32_t{ 0 }; levelIt < levelCount; ++levelIt)
			{
				auto const& level = image.levels[levelIt];
				auto const levelByteSize = static_cast<std::streamsize>(level.dataSize);

				a_stream.seekg(static_cast<std::streamoff>(levelIndex[levelIt].byteOffset));
				a_stream.read(reinterpret_cast<char*>(image.data.data() + level.dataOffset), levelByteSize);
				if (!VOB_AOE_CHECK_LOG(
					a_stream.gcount() == levelByteSize, "Truncated ktx2 level {}.", levelIt))
				{
					return ImageData{};
				}
			}

			return image;
		}

		ImageData loadStbImage(std::istream& a_stream)
		{
			stbi_io_callbacks callbacks;
			callbacks.read = [](void* user, char* data, int32_t size) -> int32_t
				{
					auto* stream = static_cast<std::istream*>(user);
					stream->read(data, size);
					return static_cast<int32_t>(stream->gcount());
				};

			callbacks.skip = [](void* user, int32_t n)
				{
					static_cast<std::istream*>(user)->seekg(n, std::ios::cur);
				};

			callbacks.eof = [](void* user) -> int32_t
				{
					return static_cast<std::istream*>(user)->eof() ? 1 : 0;
				};

			static constexpr std::array<ImageData::Format, 4> k_channelCountFormats = {
				ImageData::Format::R8, ImageData::Format::Rg8, ImageData::Format::Rgb8, ImageData::Format::Rgba8 };

			int32_t width, height, channelCount;
			uint8_t* data = stbi_load_from_callbacks(&callbacks, &a_stream, &width, &height, &channelCount, 0 /* required channel count */);

			if (!VOB_AOE_CHECK_LOG(data != nullptr, "Failed to load image.")
				|| !VOB_AOE_CHECK_LOG(1 <= channelCount && channelCount <= 4, "Invalid image channel count."))
			{
				stbi_image_free(data);
				return ImageData{};
			}

			auto const dataSize = static_cast<size_t>(width) * height * channelCount;
			auto image = ImageData{
				.format = k_channelCountFormats[channelCount - 1],
				.levels = { ImageData::Level{ glm::uvec2{ width, height }, 0, dataSize } },
				.data = std::pmr::vector<uint8_t>(data, data + dataSize)
			};
			stbi_image_free(data);

			return image;
		}
	}

	ImageData ImageLoader::load(std::istream& a_stream) const
	{
		return isKtx2(a_stream) ? loadKtx2Image(a_stream) : loadStbImage(a_stream);
	}
}
