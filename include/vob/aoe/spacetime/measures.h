#pragma once

#include <vob/misc/physics/measure.h>

#include <glm/glm.hpp>

namespace vob::aoest
{
	using length_vector = glm::vec<3, misph::measure_length>;
	using velocity_vector = glm::vec<3, misph::measure_velocity>;
	using acceleration_vector = glm::vec<3, misph::measure_acceleration>;

	inline length_vector get_translation(glm::mat4 const& a_matrix)
	{
		return a_matrix[3];
	}
}
