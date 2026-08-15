#include "vob/aoe/rendering/data/ImageLoader.h"

#include "vob/aoe/debug/Check.h"

#include "stb_image.h"

#include <array>
#include <span>


namespace vob::aoegl
{
	ImageData ImageLoader::load(std::istream& a_stream) const
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
