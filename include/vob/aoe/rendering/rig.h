#pragma once

#include <glm/glm.hpp>

#include <array>


namespace vob::aoegl
{
	static constexpr std::size_t k_rigMaxSize = 64;

	using rig = std::array<glm::mat4, k_rigMaxSize>;

	constexpr rig create_default_rig()
	{
		rig defaultRig{};
		for (auto i = 0u; i < k_rigMaxSize; ++i)
		{
			defaultRig[i] = glm::mat4{ 1.0f };
		}
		return defaultRig;
	}
}
