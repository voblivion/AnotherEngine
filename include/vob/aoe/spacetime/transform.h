#pragma once

#include <vob/aoe/data/glm_accept.h>

#include <vob/misc/std/message_macros.h>
#include <vob/misc/visitor/macros.h>

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


namespace vob::aoest
{
	struct position : public glm::vec3
	{
		template <typename... TArgs>
		position(TArgs&&... a_args)
			: glm::vec3{ std::forward<TArgs>(a_args)... }
		{}
	};

	struct rotation : public glm::quat
	{
		template <typename... TArgs>
		rotation(TArgs&&... a_args)
			: glm::quat{ std::forward<TArgs>(a_args)... }
		{}
	};

	inline glm::mat4 combine(glm::vec3 const& a_position, glm::quat const& a_rotation)
	{
		auto rotationMatrix = glm::mat4_cast(a_rotation);
		rotationMatrix[3] = glm::vec4(a_position, 1.0f);
		return rotationMatrix;
	}

	inline glm::vec3 get_position(glm::mat4 const& a_transform)
	{
		return a_transform[3];
	}

	inline glm::quat get_rotation(glm::mat4 const& a_transform)
	{
		return glm::quat_cast(a_transform);
	}
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoest::position)
	{
		a_visitor.visit(nvp("X", a_value.x));
		a_visitor.visit(nvp("Y", a_value.y));
		a_visitor.visit(nvp("Z", a_value.z));
		return true;
	}

	VOB_MISVI_ACCEPT(aoest::rotation)
	{
		a_visitor.visit(nvp("X", a_value.x));
		a_visitor.visit(nvp("Y", a_value.y));
		a_visitor.visit(nvp("Z", a_value.z));
		a_visitor.visit(nvp("W", a_value.w));
		return true;
	}
}
