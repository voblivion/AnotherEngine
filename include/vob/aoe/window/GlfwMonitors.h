#pragma once

#include "vob/aoe/window/Monitors.h"

namespace vob::aoewi
{
	class GlfwMonitors final : public IMonitors
	{
	public:
		// monitor collection
		int32_t getCount() const override;
		std::optional<int32_t> getPrimaryIndex() const override;

		// find monitor
		std::optional<int32_t> getIndex(std::string_view a_id) const override;
		std::optional<int32_t> getIndexAt(glm::ivec2 a_position) const override;
		std::optional<int32_t> resolveIndex(
			std::string_view a_name, std::string_view a_id, int32_t a_index) const override;

		// monitor properties
		std::string getId(int32_t a_index) const override;
		std::string getName(int32_t a_index) const override;
		glm::ivec2 getResolution(int32_t a_index) const override;
		glm::ivec2 getPosition(int32_t a_index) const override;
		std::vector<glm::ivec2> getSupportedResolutions(int32_t a_index) const override;
	};
}
