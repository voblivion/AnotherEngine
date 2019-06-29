#pragma once

#include <aoe/core/visitor/Utils.h>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/quaternion.hpp>
#include <aoe/common/space/Vector.h>

namespace aoe
{
	namespace common
	{
		using Quaternion = glm::quat;

		inline bool isNullWithEpsilon(Quaternion const& a_quaternion
			, float const a_epsilon = FLT_EPSILON)
		{
			return a_quaternion.x * a_quaternion.x + a_quaternion.y * a_quaternion.y
				+ a_quaternion.z * a_quaternion.z < FLT_EPSILON * FLT_EPSILON;
		}
	}

	namespace visitor
	{
		template <typename VisitorType
			, std::enable_if_t<VisitorType::accessType == AccessType::Writer>* = nullptr>
		void makeVisit(VisitorType& a_visitor, common::Quaternion& a_quaternion)
		{
			common::Vector3 t_eulerAngles{};
			a_visitor.visit(t_eulerAngles);
			a_quaternion = common::Quaternion{ t_eulerAngles };
		}

		template <typename VisitorType
			, std::enable_if_t<VisitorType::accessType == AccessType::Reader>* = nullptr>
			void makeVisit(VisitorType& a_visitor, common::Quaternion& a_quaternion)
		{
			auto const t_eulerAngles = glm::eulerAngles(a_quaternion);
			a_visitor.visit(t_eulerAngles);
		}
	}
}
