#include "vob/aoe/rendering/ImageLoader.h"

#include "stb_image.h"

#include <span>


namespace vob::aoegl
{
	Image ImageLoader::load(std::istream& a_stream) const
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

		int32_t width, height, channelCount;
		uint8_t* data = stbi_load_from_callbacks(&callbacks, &a_stream, &width, &height, &channelCount, 0 /* required channel count */);

		auto image = Image{
			.channelCount = channelCount,
			.size = glm::uvec2{ width, height },
			.data = std::pmr::vector<uint8_t>(data, data + width * height * channelCount)
		};
		stbi_image_free(data);

		return image;
	}
}
