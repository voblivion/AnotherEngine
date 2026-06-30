#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"


namespace vob::aoema
{
	template <typename T>
	T normalizeWithDefault(T const& a_vector, T const& a_defaultVector)
	{
		auto const length2 = glm::length2(a_vector);
		return length2 > glm::epsilon<float>() ? a_vector / std::sqrt(length2) : a_defaultVector;
	}
}
