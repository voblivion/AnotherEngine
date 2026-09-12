#pragma once

#include "glm/glm.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace vob::aoewi
{
	struct IMonitors
	{
		virtual ~IMonitors() = default;

		virtual int32_t getCount() const = 0;
		virtual std::optional<int32_t> getPrimaryIndex() const = 0;

		virtual std::optional<int32_t> getIndex(std::string_view a_id) const = 0;
		virtual std::optional<int32_t> getIndexAt(glm::ivec2 a_position) const = 0;
		virtual std::optional<int32_t> resolveIndex(
			std::string_view a_name, std::string_view a_id, int32_t a_index) const
			= 0;

		virtual std::string getId(int32_t a_index) const = 0;
		virtual std::string getName(int32_t a_index) const = 0;
		virtual glm::ivec2 getResolution(int32_t a_index) const = 0;
		virtual glm::ivec2 getPosition(int32_t a_index) const = 0;
		virtual std::vector<glm::ivec2> getSupportedResolutions(int32_t a_index) const = 0;
	};
}
